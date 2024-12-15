/*
 * buttons.h
 *
 * Author: Peter Sutton
 *
 * Functions and definitions for interacting with the push buttons. It is
 * assumed that buttons B0 - B3 are connected to pins B0 - B3.
 */ 

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

// Number of buttons.
#define NUM_BUTTONS 4

// Button states.
typedef enum
{
	NO_BUTTON_PUSHED = -1,
	BUTTON0_PUSHED = 0,
	BUTTON1_PUSHED = 1,
	BUTTON2_PUSHED = 2,
	BUTTON3_PUSHED = 3
} ButtonState;

/// <summary>
/// Sets up pin change interrupts on pins B0 to B3. It is assumed that
/// global interrupts are off when this function is called and are enabled
/// sometime after this function is called. This function should only be
/// called once.
/// </summary>
void init_buttons(void);

/// <summary>
/// Gets the last button pushed. A small queue of button pushes is kept.
/// This function should be called frequently enough to ensure the queue
/// does not overflow. Excess button pushes are discarded.
/// </summary>
/// <returns>The last button pushed (BUTTONx_PUSHED), or NO_BUTTON_PUSHED
/// if there are no button pushes to return.</returns>
ButtonState button_pushed(void);

/// <summary>
/// Clears all buffered button presses.
/// </summary>
void clear_button_presses(void);

#endif /* BUTTONS_H_ */
