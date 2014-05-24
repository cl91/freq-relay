/*
 * state_machine.c
 *
 *  Created on: 16/04/2014
 *      Author: cliu712
 */

#include <stdbool.h>
#include <stdio.h>

// forward declarations
bool timer_expired(void);
void reset_timer(void);
bool is_network_unstable(void);
bool try_shed_load(void);
bool try_reconnect(void);

// We do not need synchronisation here, as data access from a task is limited to a simple read.
// The only read-write cycle is in the button ISR, which is effectively a big synchronisation lock.
static bool is_maintenance = false;

static inline bool is_maintenance_state(void)
{
	return is_maintenance;
}

static inline void toggle_maintenance_state(void)
{
	is_maintenance = !is_maintenance;
}

enum states {
	STATE_STABLE,		// network stable, try detecting instability
	STATE_RECONNECT,	// network stable enough, try reconnecting load, will go to stable if all connected
	STATE_SHED_LOAD,	// network unstable, try to shed load,
						// do nothing if all disconnected, goes to monitor unstable
	STATE_MONITOR_UNSTABLE,		// network unstable, monitoring network,
								// will shed load if remains unstable for 0.5sec,
								// will go to monitor stable if fails to detect instability
	STATE_MONITOR_STABLE		// network stable, monitoring network, will reconnect if stable for 0.5sec
								// will go to monitor unstable if detects instability
};

void state_machine_task(void *arg)
{
	enum states state = STATE_STABLE;

	for (;;) {
		if (is_maintenance_state()) { // maintenance state --- do nothing
			continue;
		}
		switch (state) {
		case STATE_STABLE:
			if (is_network_unstable()) {
				state = STATE_SHED_LOAD;
			} else {
				state = STATE_STABLE;
			}
			break;
		case STATE_RECONNECT:
			if (try_reconnect()) {
				reset_timer();
				state = STATE_MONITOR_STABLE;
			} else {	// all loads are reconnected, goes to stable state
				state = STATE_STABLE;
			}
			break;
		case STATE_SHED_LOAD:
			try_shed_load();
			reset_timer();
			state = STATE_MONITOR_UNSTABLE;
			break;
		case STATE_MONITOR_UNSTABLE:
			if (timer_expired()) {
				reset_timer();
				state = STATE_SHED_LOAD;
			} else if (is_network_unstable()) {
				state = STATE_MONITOR_UNSTABLE;
			} else {
				reset_timer();
				state = STATE_MONITOR_STABLE;
			}
			break;
		case STATE_MONITOR_STABLE:
			if (timer_expired()) {
				reset_timer();
				state = STATE_RECONNECT;
			} else if (is_network_unstable()) {
				reset_timer();
				state = STATE_MONITOR_UNSTABLE;
			} else {
				state = STATE_MONITOR_STABLE;
			}
			break;
		}
	}
}

#include "system.h"						// to use the symbolic names
#include "altera_avalon_pio_regs.h"		// to use PIO functions
#include "alt_types.h"					// alt_u32 is a kind of alt_types
#include "sys/alt_irq.h"				// to register interrupts

// push a button to enter or exit maintenance state
void button_interrupts_function(void *ignored, alt_u32 id)
{
	toggle_maintenance_state();

	// clears the edge capture register
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PUSH_BUTTON_BASE, 0x7);
}

void init_button(void)
{
	  // clears the edge capture register. Writing 1 to bit clears pending interrupt for corresponding button.
	  IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PUSH_BUTTON_BASE, 0x7);

	  // enable interrupts for all buttons
	  IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PUSH_BUTTON_BASE, 0x7);

	  // register the ISR
	  alt_irq_register(PUSH_BUTTON_IRQ, NULL, button_interrupts_function);
}

/* For xTaskCreate. */
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"

void init_state_machine(void)
{
	init_button();
	xTaskCreate(state_machine_task, "state_machine", configMINIMAL_STACK_SIZE, NULL,
				tskIDLE_PRIORITY+2, NULL);
}
