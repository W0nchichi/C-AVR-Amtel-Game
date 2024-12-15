/*
 * ledmatrix.h
 *
 * Author: Peter Sutton
 *
 * Functions and definitions for interacting with the LED matrix via SPI.
 * These should be used to encapsulate all sending of SPI commands.
 */

#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_

#include <stdint.h>
#include "pixel_colour.h"

// The matrix has 8 rows (0 - 7, bottom to top) and 16 columns (0 - 15,
// left to right).
#define MATRIX_NUM_ROWS   	(8)
#define MATRIX_NUM_COLUMNS	(16)

// Data types which can be used to store display information.
typedef PixelColour MatrixData[MATRIX_NUM_ROWS][MATRIX_NUM_COLUMNS];
typedef PixelColour MatrixRow[MATRIX_NUM_COLUMNS];
typedef PixelColour MatrixColumn[MATRIX_NUM_ROWS];

/// <summary>
/// Sets up the LED matrix. This function must be called before any of
/// the other LED matrix functions can be used. This function should only
/// be called once.
/// </summary>
void init_ledmatrix(void);


//
// Functions to update the display.
//

/// <summary>
/// Updates all pixels of the LED matrix.
/// </summary>
/// <param name="data">New colours for all pixels of the LED matrix.</param>
void ledmatrix_update_all(MatrixData data);

/// <summary>
/// Updates a specific pixel of the LED matrix.
/// </summary>
/// <param name="row">The row number of the pixel.</param>
/// <param name="col">The column number of the pixel.</param>
/// <param name="pixel">New colour of the pixel.</param>
void ledmatrix_update_pixel(uint8_t row, uint8_t col, PixelColour pixel);

/// <summary>
/// Updates a row of the LED matrix.
/// </summary>
/// <param name="row">The row to update.</param>
/// <param name="data">New colours for the row.</param>
void ledmatrix_update_row(uint8_t row, MatrixRow data);

/// <summary>
/// Updates a column of the LED matrix.
/// </summary>
/// <param name="col">The column to update.</param>
/// <param name="data">New colours for the column.</param>
void ledmatrix_update_column(uint8_t col, MatrixColumn data);

/// <summary>
/// Shifts the entire LED matrix to the left by one column.
/// </summary>
void ledmatrix_shift_display_left(void);

/// <summary>
/// Shifts the entire LED matrix to the right by one column.
/// </summary>
void ledmatrix_shift_display_right(void);

/// <summary>
/// Shifts the entire LED matrix up by one row.
/// </summary>
void ledmatrix_shift_display_up(void);

/// <summary>
/// Shifts the entire LED matrix down by one row.
/// </summary>
void ledmatrix_shift_display_down(void);

/// <summary>
/// Clears the entire LED matrix.
/// </summary>
void ledmatrix_clear(void);

//
// Functions to operate on MatrixRow and MatrixColumn data structures.
//

/// <summary>
/// Copies pixel colours from one MatrixColumn to another.
/// </summary>
/// <param name="from">The source MatrixColumn.</param>
/// <param name="to">The destination MatrixColumn.</param>
void copy_matrix_column(MatrixColumn from, MatrixColumn to);

/// <summary>
/// Copies pixel colours from one MatrixRow to another.
/// </summary>
/// <param name="from">The source MatrixRow.</param>
/// <param name="to">The destination MatrixRow.</param>
void copy_matrix_row(MatrixRow from, MatrixRow to);

/// <summary>
/// Sets the pixel colours of a MatrixColumn.
/// </summary>
/// <param name="matrix_column">The MatrixColumn.</param>
/// <param name="colour">The colour.</param>
void set_matrix_column_to_colour(MatrixColumn matrix_column,
        PixelColour colour);

/// <summary>
/// Sets the pixel colours of a MatrixRow.
/// </summary>
/// <param name="matrix_row">The MatrixRow.</param>
/// <param name="colour">The colour.</param>
void set_matrix_row_to_colour(MatrixRow matrix_row, PixelColour colour);

#endif /* LEDMATRIX_H_ */
