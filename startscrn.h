/*
 * startscrn.h
 *
 * Author: Yufeng Gao
 *
 * Functions for displaying the start screen animation on the LED matrix and
 * the title ASCII art on the terminal. This module is only used by the base
 * code, and you should not attempt to modify or use this module in the code
 * you write yourself.
 */ 

#ifndef STARTSCRN_H_
#define STARTSCRN_H_

#include <stdint.h>

/// <summary>
/// Sets up the start screen on the LED matrix. This function must be called
/// before the start screen may be updated.
/// </summary>
void setup_start_screen(void);

/// <summary>
/// Updates the start screen on the LED matrix.
/// </summary>
void update_start_screen(void);

/// <summary>
/// Draws the terminal title ASCII art.
/// </summary>
/// <param name="row">The start row of the ASCII art.</param>
/// <param name="col">The start column of the ASCII art.</param>
void display_terminal_title(uint8_t row, uint8_t col);

#endif /* STARTSCRN_H_ */
