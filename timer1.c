/*
 * timer1.c
 *
 * Author: Peter Sutton
 */

#include "timer1.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void init_timer1(void)
{
	// Setup timer 1.
	TCNT1 = 0;
}
