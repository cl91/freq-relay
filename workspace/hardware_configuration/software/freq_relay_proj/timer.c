/*
 * timer.c
 *
 *  Created on: 16/04/2014
 *      Author: cliu712
 */

#include <stdbool.h>
#include <stdio.h>
#include "FreeRTOS/FreeRTOS.h"
#include "FreeRTOS/semphr.h"
#include "FreeRTOS/timers.h"
#include "FreeRTOS/portmacro.h"
#include "panic.h"

// is_timer_expired does not need protection from a semaphore, as it does not have inconsistent
// intermediate states during data access (data write and read are atomic for native machine word)
// is_timer_expired is used to indicate (to the main
// state machine task) that the 0.5sec timer has expired
bool is_timer_expired;

static xTimerHandle timer;

// timer expires after 500ms
#define TIMER_PERIOD	(500 / portTICK_RATE_MS)

// forward declaration
void timer_callback(xTimerHandle);

void init_timer(void)
{
	timer = xTimerCreate("timer", TIMER_PERIOD, pdFALSE, NULL, timer_callback);
}

void timer_callback(xTimerHandle timer)
{
	is_timer_expired = true;
}

bool timer_expired(void)
{
	return is_timer_expired;
}

void reset_timer(void)
{
	is_timer_expired = false;
	xTimerReset(timer, 10);
}
