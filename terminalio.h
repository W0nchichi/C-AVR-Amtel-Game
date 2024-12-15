/*
 * terminalio.h
 *
 * Author: Peter Sutton
 *
 * Functions and definitions for interacting with the terminal. These should
 * be used to encapsulate all sending of escape sequences.
 */

#ifndef TERMINAL_IO_H_
#define TERMINAL_IO_H_

#include <stdint.h>

/*
	Column number and row number are measured relative to the top
	left of the screen. First column is 0, first row is 0.

	The display parameter is a number between 0 and 47. Valid values are
	                                Foreground colours  Background colours
	                                ------------------  ------------------
	    0 Reset all attributes      30 Black            40 Black
	    1 Bright                    31 Red              41 Red
	    2 Dim                       32 Green            42 Green
	    4 Underscore                33 Yellow           43 Yellow
	    5 Blink                     34 Blue             44 Blue
	    7 Reverse Video             35 Magenta          45 Magenta
	    8 Hidden                    36 Cyan             46 Cyan
	                                37 White            47 White
*/
typedef enum
{
	TERM_RESET = 0,
	TERM_BRIGHT = 1,
	TERM_DIM = 2,
	TERM_UNDERSCORE = 4,
	TERM_BLINK = 5,
	TERM_REVERSE = 7,
	TERM_HIDDEN = 8,
	FG_BLACK = 30,
	FG_RED = 31,
	FG_GREEN = 32,
	FG_YELLOW= 33,
	FG_BLUE = 34,
	FG_MAGENTA = 35,
	FG_CYAN = 36,
	FG_WHITE = 37,
	BG_BLACK = 40,
	BG_RED = 41,
	BG_GREEN = 42,
	BG_YELLOW = 43,
	BG_BLUE = 44,
	BG_MAGENTA = 45,
	BG_CYAN = 46,
	BG_WHITE = 47
} DisplayParameter;

/// <summary>
/// Moves the terminal cursor to a new location. Row and column numbers use
/// 0-based indexing.
/// </summary>
/// <param name="row">The new row number of the terminal cursor.</param>
/// <param name="col">The new column number of the terminal cursor.</param>
void move_terminal_cursor(int row, int col);

/// <summary>
/// Resets the terminal display mode.
/// </summary>
void normal_display_mode(void);

/// <summary>
/// Sets the terminal display mode to reverse video.
/// </summary>
void reverse_video(void);

/// <summary>
/// Clears the terminal.
/// </summary>
void clear_terminal(void);

/// <summary>
/// Clears to the end of the row the cursor is on.
/// </summary>
void clear_to_end_of_line(void);

/// <summary>
/// Sets a display attribute.
/// </summary>
/// <param name="parameter">The display attribute to set.</param>
void set_display_attribute(DisplayParameter parameter);

/// <summary>
/// Hides the blinking terminal cursor from the user.
/// </summary>
void hide_cursor(void);

/// <summary>
/// Shows the blinking terminal cursor to the user.
/// </summary>
void show_cursor(void);

/// <summary>
/// Enables scrolling for the entire terminal.
/// </summary>
void enable_scrolling_for_whole_display(void);

/// <summary>
/// Sets a custom scroll region.
/// </summary>
/// <param name="row1">The top row of the region, inclusive.</param>
/// <param name="row2">The bottom row of the region, inclusive.</param>
void set_scroll_region(int row1, int row2);

/// <summary>
/// Scrolls the scroll region of the terminal down. If the cursor is in the
/// first (top) row of the scroll region then scroll the scroll region down by
/// one row. The bottom row of the scroll region will be lost. The top row of
/// the scroll region will be blank. If the cursor is not in the first row of
/// the scroll regionm then the cursor will just be moved up by one row.
/// </summary>
void scroll_down(void);

/// <summary>
/// Scrolls the scroll region of the terminal up. If the cursor is in the last
/// (bottom) row of the scroll region then scroll the scroll region up by one
/// row. The top row of the scroll region will be lost. The bottom row of the
/// scroll region will be blank. If the cursor is not in the last row of the
/// scroll region, then cursor will just be moved down by one row.
/// </summary>
void scroll_up(void);

/// <summary>
/// Draws a white horizontal line on the terminal. Row and column numbers use
/// 0-based indexing.
/// </summary>
/// <param name="row">The row to draw the line on.</param>
/// <param name="start_col">The start column of the line, inclusive.</param>
/// <param name="end_col">The end column of the line, inclusive.</param>
void draw_horizontal_line(int row, int start_col, int end_col);

/// <summary>
/// Draws a white vertical line on the terminal. Row and column numbers use
/// 0-based indexing.
/// </summary>
/// <param name="col">The column to draw the line on.</param>
/// <param name="start_row">The start row of the line, inclusive.</param>
/// <param name="end_row">The end row of the line, inclusive.</param>
void draw_vertical_line(int col, int start_row, int end_row);

#endif /* TERMINAL_IO_H */
