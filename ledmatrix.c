/*
 * ledmatrix.c
 *
 * Author: Peter Sutton
 */

#include "ledmatrix.h"
#include <stdint.h>
#include "spi.h"
#include "pixel_colour.h"

#define CMD_UPDATE_ALL		(0x00)
#define CMD_UPDATE_PIXEL	(0x01)
#define CMD_UPDATE_ROW		(0x02)
#define CMD_UPDATE_COL		(0x03)
#define CMD_SHIFT_DISPLAY	(0x04)
#define CMD_CLEAR_SCREEN	(0x0F)

void init_ledmatrix(void)
{
	// Setup SPI, with a clock devider of 128. This speed guarantees the
	// SPI buffer will never overflow on the LED matrix.
	spi_setup_master(128);
}

void ledmatrix_update_all(MatrixData data)
{
	(void)spi_send_byte(CMD_UPDATE_ALL);
	for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)
	{
		for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
		{
			(void)spi_send_byte(data[row][col]);
		}
	}
}

void ledmatrix_update_pixel(uint8_t row, uint8_t col, PixelColour pixel)
{
	if (col >= MATRIX_NUM_COLUMNS || row >= MATRIX_NUM_ROWS)
	{
		// Invalid location, ignore the request.
		return;
	}
	(void)spi_send_byte(CMD_UPDATE_PIXEL);
	(void)spi_send_byte(((row & 0x07) << 4) | (col & 0x0F));
	(void)spi_send_byte(pixel);
}

void ledmatrix_update_row(uint8_t row, MatrixRow data)
{
	if (row >= MATRIX_NUM_ROWS)
	{
		// Invalid row number, ignore the request.
		return;
	}
	(void)spi_send_byte(CMD_UPDATE_ROW);
	(void)spi_send_byte(row & 0x07);
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		(void)spi_send_byte(data[col]);
	}
}

void ledmatrix_update_column(uint8_t col, MatrixColumn data)
{
	if (col >= MATRIX_NUM_COLUMNS)
	{
		// Invalid column number, ignore the request.
		return;
	}
	(void)spi_send_byte(CMD_UPDATE_COL);
	(void)spi_send_byte(col & 0x0F);
	for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)
	{
		(void)spi_send_byte(data[row]);
	}
}

void ledmatrix_shift_display_left(void)
{
	(void)spi_send_byte(CMD_SHIFT_DISPLAY);
	(void)spi_send_byte(0x02);
}

void ledmatrix_shift_display_right(void)
{
	(void)spi_send_byte(CMD_SHIFT_DISPLAY);
	(void)spi_send_byte(0x01);
}

void ledmatrix_shift_display_up(void)
{
	(void)spi_send_byte(CMD_SHIFT_DISPLAY);
	(void)spi_send_byte(0x08);
}

void ledmatrix_shift_display_down(void)
{
	(void)spi_send_byte(CMD_SHIFT_DISPLAY);
	(void)spi_send_byte(0x04);
}

void ledmatrix_clear(void)
{
	(void)spi_send_byte(CMD_CLEAR_SCREEN);
}

void copy_matrix_column(MatrixColumn from, MatrixColumn to)
{
	for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)
	{
		to[row] = from[row];
	}
}

void copy_matrix_row(MatrixRow from, MatrixRow to)
{
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++)
	{
		to[col] = from[col];
	}
}

void set_matrix_column_to_colour(MatrixColumn matrix_column,
	PixelColour colour)
{
	for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)
	{
		matrix_column[row] = colour;
	}
}

void set_matrix_row_to_colour(MatrixRow matrix_row, PixelColour colour)
{
	for (uint8_t column = 0; column < MATRIX_NUM_COLUMNS; column++)
	{
		matrix_row[column] = colour;
	}
}
