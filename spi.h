/*
 * spi.h
 *
 * Author: Peter Sutton
 *
 * Functions for sending and receiving data via SPI. This module is only
 * used by the base code, and you should not attempt to modify or use this
 * module in the code you write yourself.
 */ 

#ifndef SPI_H_
#define SPI_H_

#include <stdint.h>

/// <summary>
/// Sets up SPI communication as a master. This function must be called
/// before any of the SPI functions can be used. This function should only
/// be called once.
/// </summary>
/// <param name="clockdivider">The clock divider, should be one of 2, 4, 8,
/// 16, 32, 64, 128.</param>
void spi_setup_master(uint8_t clockdivider);

/// <summary>
/// Sends and receives an SPI byte. This function will take at least 8 
/// cycles of the divided clock (i.e. will busy wait).
/// </summary>
/// <param name="byte">The byte to send.</param>
/// <returns>The byte received.</returns>
uint8_t spi_send_byte(uint8_t byte);

#endif /* SPI_H_ */
