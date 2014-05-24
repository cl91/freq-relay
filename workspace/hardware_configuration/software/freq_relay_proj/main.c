/* Standard includes. */
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/task.h"

// forward declaration
void init_timer(void);
void init_freq_init(void);
void init_vga(void);
void init_state_machine(void);

// Initialise all subsystems and start the scheduler
int main(void)
{
	init_timer();
	init_freq_irq();
	init_vga();
	init_state_machine();

	/* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	 the scheduler. */
	for (;;);
}
