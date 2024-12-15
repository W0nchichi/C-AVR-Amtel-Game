/*
 * timer0.h
 *
 * Author: Peter Sutton
 *
 * Module for the system clock, and function(s) for getting the current time.
 * Timer 0 is setup to generate an interrupt every millisecond. Tasks that
 * have to occur regularly (every millisecond or few) can be added to the
 * interrupt handler (in timer0.c) or can be added to the main event loop that
 * checks the clock tick value. Any tasks undertaken in the interrupt handler
 * should be kept short so that we don't run the risk of missing an interrupt
 * in future.
 */

#ifndef TIMER0_H_
#define TIMER0_H_

#include <stdint.h>

/// <summary>
/// Initialises timer 0 for system clock. An interrupt will be generated
/// every millisecond to update the time reference. This function must be
/// called before any of the other timer 0 functions can be used. This
/// function should only be called once.
/// </summary>
void init_timer0(void);

/// <summary>
/// Gets the current time (milliseconds since the timer was initialised).
/// </summary>
/// <returns>Milliseconds since timer 0 was initialised.</returns>
uint32_t get_current_time(void);

#endif /* TIMER0_H_ */
