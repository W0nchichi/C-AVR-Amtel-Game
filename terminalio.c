/*
 * terminalio.c
 *
 * Author: Peter Sutton
 */

#include "terminalio.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <avr/pgmspace.h>

void move_terminal_cursor(int row, int col)
{
    printf_P(PSTR("\x1b[%d;%dH"), row + 1, col + 1);
}

void normal_display_mode(void)
{
	printf_P(PSTR("\x1b[0m"));
}

void reverse_video(void)
{
	printf_P(PSTR("\x1b[7m"));
}

void clear_terminal(void)
{
	printf_P(PSTR("\x1b[2J"));
}

void clear_to_end_of_line(void)
{
	printf_P(PSTR("\x1b[K"));
}

void set_display_attribute(DisplayParameter parameter)
{
	printf_P(PSTR("\x1b[%dm"), parameter);
}

void hide_cursor(void)
{
	printf_P(PSTR("\x1b[?25l"));
}

void show_cursor(void)
{
	printf_P(PSTR("\x1b[?25h"));
}

void enable_scrolling_for_whole_display(void)
{
	printf_P(PSTR("\x1b[r"));
}

void set_scroll_region(int row1, int row2)
{
	printf_P(PSTR("\x1b[%d;%dr"), row1 + 1, row2 + 1);
}

void scroll_down(void)
{
	printf_P(PSTR("\x1bM")); // ESC-M
}

void scroll_up(void)
{
	printf_P(PSTR("\x1b\x44")); // ESC-D
}

void draw_horizontal_line(int row, int start_col, int end_col)
{
	// Place cursor at starting position.
	move_terminal_cursor(row, start_col);
	// Reverse the video - black on white.
	reverse_video();
	// Print spaces until the end column. Since spaces are blank,
	// and we're in reverse video mode, a fat white line gets drawn.
	for (int i = start_col; i <= end_col; i++)
	{
		putchar(' '); // Print space.
	}
	// Reset the mode to normal.
	normal_display_mode();
}

void draw_vertical_line(int col, int start_row, int end_row)
{
	// Place cursor at starting position.
	move_terminal_cursor(start_row, col);
	// Reverse the video - black on white.
	reverse_video();
	// Print spaces until the row above end row. Since spaces are blank,
	// and we're in reverse video mode, a fat white line gets drawn.
	for (int i = start_row; i < end_row; i++)
	{
		putchar(' '); // Print space.
		// Move down a row and step back to previous column (because
		// printing the space caused the cursor to be advanced by one
		// column).
		printf_P(PSTR("\x1b[B\x1b[D"));
	}
	// Print the space for the end row, and do not move the cursor down.
	putchar(' ');
	// Reset the mode to normal.
	normal_display_mode();
}
