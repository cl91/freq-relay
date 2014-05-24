/*
 * freq_irq.c
 *
 *  Created on: 16/04/2014
 *      Author: cliu712
 */

#include <stdbool.h>
#include <stdio.h>
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/queue.h"
#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include "panic.h"

// forward declaration
void init_load_control(void);
void init_threshold_control(void);
bool is_off_threshold(double freq, double roc);

// message structure in queue
struct message {
	double freq;
	double roc;
};

// for passing freq and roc data around
static xQueueHandle freq_queue;

void post_freq_to_vga_buffer(struct message);
// must be called from task context
bool is_network_unstable(void)
{
	struct message data;
	xQueueReceive(freq_queue, &data, portMAX_DELAY);
	post_freq_to_vga_buffer(data);
	return is_off_threshold(data.freq, data.roc);
}

void do_freq_irq(void *p, alt_u32 id)
{
	static double old_freq = -1;
	static int old_count = 0;
	unsigned int new_count = IORD(FREQUENCY_ANALYSER_BASE, 0);	// number of ADC samples
	double new_freq = 16000/(double)new_count;

	int ave_count = (new_count + old_count) / 2;	// average number of samples
	double roc = (new_freq - old_freq) * 16000 / ave_count;		// rate of change

	// need at least two points for roc value, therefore on 1st run of
	// of this function roc value is invalid.
	if (old_freq < 0)
		roc = 0;

	struct message data = { new_freq, roc };
	portBASE_TYPE ignored;
	xQueueSendFromISR(freq_queue, &data, &ignored);

	old_freq = new_freq;
	old_count = new_count;
}

void init_freq_irq(void)
{
	init_load_control();
	freq_queue = xQueueCreate(1, sizeof(struct message));
	if (freq_queue == NULL)
		panic("xQueueCreate failed");
	init_threshold_control();
	alt_irq_register(FREQUENCY_ANALYSER_IRQ, 0, do_freq_irq);
}
