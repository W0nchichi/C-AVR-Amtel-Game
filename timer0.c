/*
 * timer0.c
 *
 * Author: Peter Sutton
 */

#include "timer0.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Our internal clock tick count - incremented every millisecond. Will
// overflow every ~49 days.
static volatile uint32_t clock_ticks_ms;

void init_timer0(void)
{
	// Reset clock tick count. L indicates a long (32 bit) constant.
	clock_ticks_ms = 0L;

	// Set up timer 0 to generate an interrupt every 1ms. We will divide
	// the clock by 64 and count up to 124. We will therefore get an
	// interrupt every 64 x 125 clock cycles, i.e. every 1 milliseconds
	// with an 8MHz clock. The counter will be reset to 0 when it reaches
	// it's output compare value.

	// Clear the timer.
	TCNT0 = 0;

	// Set the output compare value to be 124.
	OCR0A = 124;

	// Set the timer to clear on compare match (CTC mode) and to
	// divide the clock by 64. This starts the timer running.
	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS01) | (1 << CS00);

	// Enable an interrupt on output compare match. Note that
	// interrupts have to be enabled globally before the interrupts
	// will fire.
	TIMSK0 |= (1 << OCIE0A);

	// Make sure the interrupt flag is cleared by writing a 1 to it.
	TIFR0 = (1 << OCF0A);
}

uint32_t get_current_time(void)
{
	// Disable interrupts so we can be sure that the interrupt doesn't
	// fire when we've copied just a couple of bytes of the value.
	// Interrupts are re-enabled if they were enabled at the start.
	uint8_t interrupts_were_enabled = bit_is_set(SREG, SREG_I);
	cli();
	uint32_t result = clock_ticks_ms;
	if (interrupts_were_enabled)
	{
		sei();
	}
	return result;
}

// Interrupt handler for clock tick.
ISR(TIMER0_COMPA_vect)
{
	// Increment our clock tick count.
	clock_ticks_ms++;
}
