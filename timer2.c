/*
 * timer2.c
 *
 * Author: Peter Sutton
 */

#include "timer2.h"
#include <avr/io.h>
#include <avr/interrupt.h>

void init_timer2(void)
{
	// Setup timer 2.
	TCNT2 = 0;
}
