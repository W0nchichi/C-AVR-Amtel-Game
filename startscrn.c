/*
 * startscrn.c
 *
 * Author: Yufeng Gao
 */ 

#include "startscrn.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/pgmspace.h>
#include "pixel_colour.h"
#include "ledmatrix.h"
#include "terminalio.h"
#include "timer0.h"

// Speed definitions.
#define FLASH_TIME  	(1000)
#define FLASH_SPEED 	(100)
#define STATIC_TIME 	(1000)
#define SCROLL_SPEED	(200)

// Short colour definitions.
#define G	(COLOUR_GREEN)
#define O	(COLOUR_ORANGE)
#define D	(COLOUR_DARK_GREEN)
#define _	(COLOUR_BLACK)

// The animation data for the start screen. It is an array of MatrixColumns,
// with the 0th element being the left-most column of the start screen and the
// last element being the right-most column of the start screen. It must have
// at least MATRIX_NUM_COLUMN elements.
const MatrixColumn anim_data[] PROGMEM =
{
	{ G, G, _, G, G, G, G, _ },
	{ G, _, _, G, _, _, G, _ },
	{ G, _, _, G, _, _, G, _ },
	{ G, G, G, G, _, G, G, _ },
	{ _, _, _, _, _, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ G, _, _, _, G, _, _, _ },
	{ G, _, _, _, G, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ G, G, G, G, G, G, G, _ },
	{ _, _, _, G, _, _, _, _ },
	{ _, _, G, _, G, _, _, _ },
	{ G, G, _, _, _, G, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ G, _, _, _, G, _, _, _ },
	{ G, _, _, _, G, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ G, G, G, G, G, G, G, _ },
	{ G, _, _, G, _, _, _, _ },
	{ G, _, _, G, _, _, _, _ },
	{ G, G, G, G, _, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ G, G, G, _, G, _, _, _ },
	{ G, _, G, _, G, _, _, _ },
	{ G, _, G, _, G, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ _, _, _, _, G, _, _, _ },
	{ _, _, _, _, G, _, _, _ },
	{ G, G, G, G, G, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ O, O, O, O, O, O, _, _ },
	{ O, O, _, _, O, O, _, _ },
	{ O, _, O, O, _, O, _, _ },
	{ O, _, O, O, _, O, _, _ },
	{ O, O, _, _, O, O, _, _ },
	{ O, O, O, O, O, O, _, _ },
	{ _, _, _, D, _, D, _, _ },
	{ D, _, _, D, _, D, _, _ },
	{ _, D, _, _, D, _, D, D },
	{ _, _, D, D, _, D, D, D },
	{ _, D, _, _, _, _, D, D },
	{ D, _, _, _, _, _, _, _ },
	{ _, _, _, _, _, _, _, _ },
	{ _, _, _, _, _, _, _, _ }
};

// Undefine the short colour definitions.
#undef G
#undef O
#undef D
#undef _

// Terminal ASCII art data bits. Each element in this array represents a line
// of the ASCII art. The ASCII art has a width of 64 characters, and each bit
// in the unsigned 64-bit integer represents a column/pixel.
static const uint64_t title_data[] PROGMEM =
{
	0b1111111001111110011000110011111100111111000111110011100001100000,
	0b1100000011000011011001100110000110110001101100011011110001100000,
	0b1111111011000011011111000110000110111111001111111011011001100000,
	0b0000011011000011011001100110000110110001101100011011001101100000,
	0b1111111001111110011000110011111100111111001100011011000111100000
};

// The colour definitions for the ASCII terminal title art. The positions
// represent the columns which colour changes occur (highest to lowest), and
// each position has a corresponding terminal attribute.
const uint8_t title_pos[] = { 58, 48, 40, 32, 23, 15, 6 };
const DisplayParameter title_attr[] = { BG_CYAN, BG_WHITE, BG_RED,
	BG_YELLOW, BG_BLUE, BG_GREEN, BG_MAGENTA };

// For course staff: Code and defintions blow this point should not be
// modified unless the operation of the start screen is to be changed.
// For students reading this: Do not modify this file, if you break anything,
// you run the risk of failing this project and hence the course.

// Start screen animation flags and definitions.
static uint8_t flags;
#define FLG_IS_NEW_CYCLE 	(1U << 0) // New animation cycle next time?
#define FLG_IS_FLASH_DONE	(1U << 1) // Is flashing done?
#define FLG_TOGGLE_ON    	(1U << 2) // Toggle screen on next time?

// The time when flashing for the start screen started.
static uint32_t flashing_start_time;

// The last time the start screen was updated.
static uint32_t last_update_time;

// The column to be displayed next.
static uint8_t next_column;

// Standard min() macro if not already defined.
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

// Macro for returning the number of elements in an array.
#define countof(x) (sizeof(x) / sizeof((x)[0]))

// Macro for getting next column number.
#define GET_NEXT_COLUMN(x, d) (((x) + 1) % countof((d)))

// Displays the initial image of the start screen.
static void display_initial_image(void)
{
	for (uint8_t col = 0; col < min(MATRIX_NUM_COLUMNS,
		countof(anim_data)); col++)
	{
		MatrixColumn column_data;
		memcpy_P(column_data, &anim_data[col], sizeof(column_data));
		ledmatrix_update_column(col, column_data);
	}
}

// Displays the next column of the start screen.
static void display_next_column(void)
{
	ledmatrix_shift_display_left();
	MatrixColumn column_data;
	memcpy_P(column_data, &anim_data[next_column], sizeof(column_data));
	ledmatrix_update_column(MATRIX_NUM_COLUMNS - 1, column_data);
	next_column = GET_NEXT_COLUMN(next_column, anim_data);
	if (next_column == MATRIX_NUM_COLUMNS)
	{
		flags |= FLG_IS_NEW_CYCLE;
	}
}

void setup_start_screen(void)
{
	ledmatrix_clear();
	display_initial_image();
	flags |= FLG_IS_NEW_CYCLE;
}

void update_start_screen(void)
{
	uint32_t time = get_current_time();

	if (flags & FLG_IS_NEW_CYCLE)
	{
		flashing_start_time = time;
		last_update_time = time;
		next_column = MATRIX_NUM_COLUMNS;
		flags &= ((~FLG_IS_NEW_CYCLE) & (~FLG_IS_FLASH_DONE) &
			(~FLG_TOGGLE_ON));
	}

	if (next_column == MATRIX_NUM_COLUMNS)
	{
		if (time > flashing_start_time + FLASH_TIME + STATIC_TIME)
		{
			// Two seconds have passed since start of flashing,
			// display next column.
			display_next_column();
			last_update_time = time;
		}
		else if (!(flags & FLG_IS_FLASH_DONE) && time >
			flashing_start_time + FLASH_TIME)
		{
			// One second has passed since the start of flashing,
			// turn off flashing.
			display_initial_image();
			flags |= FLG_IS_FLASH_DONE;
			last_update_time = time;
		}
		else if (!(flags & FLG_IS_FLASH_DONE) && time >
			last_update_time + FLASH_SPEED)
		{
			// 0.1 seconds passed since last flash, flash the LED
			// matrix.
			if ((flags ^= FLG_TOGGLE_ON) & FLG_TOGGLE_ON)
			{
				ledmatrix_clear();
			}
			else
			{
				display_initial_image();
			}
			last_update_time = time;
		}
	}
	else
	{
		if (time > last_update_time + SCROLL_SPEED)
		{
			display_next_column();
			last_update_time = time;
		}
	}
}

static bool display_title_pixel(uint8_t line, uint8_t col, bool coloured)
{
	uint64_t data;
	memcpy_P(&data, &title_data[line], sizeof(data));
	if (data & ((uint64_t)1U << (63 - col)))
	{
		if (!coloured)
		{
			for (uint8_t j = 0; j < countof(title_pos); j++)
			{
				if (col <= title_pos[j])
				{
					set_display_attribute(title_attr[j]);
				}
			}
			coloured = true;
		}
	}
	else
	{
		if (coloured)
		{
			normal_display_mode();
			coloured = false;
		}
		
	}
	putchar(' ');
	return coloured;
}

void display_terminal_title(uint8_t row, uint8_t col)
{
	bool coloured = false;
	for (uint8_t line = 0; line < countof(title_data); line++)
	{
		move_terminal_cursor(line + row, col);
		for (uint8_t i = 0; i < 64; i++)
		{
			coloured = display_title_pixel(line, i, coloured);
		}
	}
}
