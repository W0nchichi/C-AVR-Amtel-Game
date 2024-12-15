/*
 * serialio.h
 *
 * Author: Peter Sutton
 * 
 * Module to allow standard input/output routines to be used via 
 * serial port 0 and functions for interacting with the input buffer.
 */

#ifndef SERIALIO_H_
#define SERIALIO_H_

#include <stdint.h>
#include <stdbool.h>

/// <summary>
/// Initialises serial I/O using the UART. This function must be called
/// before any of the standard I/O functions. This function should only
/// be called once.
/// </summary>
/// <param name="baudrate">The baud rate (e.g., 19200).</param>
/// <param name="echo">Whether inputs are echoed back.</param>
void init_serial_stdio(long baudrate, bool echo);

/// <summary>
/// Tests if input is available from the serial port. If there is
/// input available, then it can be read with a suitable standard I/O
/// library function, e.g., fgetc().
/// </summary>
/// <returns>Whether inputs are available.</returns>
bool serial_input_available(void);

/// <summary>
/// Discards any input waiting to be read from the serial port. Useful
/// for when characters may have been typed when we didn't want them.
/// </summary>
void clear_serial_input_buffer(void);

#endif /* SERIALIO_H_ */
