/*
 * serialio.c
 *
 * Author: Peter Sutton
 */

// The init_serial_stdio() function must be called before any standard IO
// functions (e.g., printf). We use interrupt-based output and a circular
// buffer to store output messages, this allows us to print many characters at
// once to the buffer and have them output by the UART as speed permits.
// If the buffer fills up, the put method will either:
//   1. Block until there is room in it, if interrupts are enabled, or
//   2. Discard the character, if interrupts are disabled.
// Input is blocking - requesting input from stdin will block until a
// character is available. If interrupts are disabled when input is sought,
// then this will block forever. The function input_available() can be used to
// test whether there is input available to read from stdin.

#include "serialio.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// System clock rate in Hz. L at the end indicates this is a long constant.
#define SYSCLK 8000000L

// Circular buffer to hold outgoing characters. The insert_pos variable keeps
// track of the position (0 to OUTPUT_BUFFER_SIZE-1) that the next outgoing
// character should be written to. bytes_in_buffer keeps count of the number
// of characters currently stored in the buffer (ranging from 0 to
// OUTPUT_BUFFER_SIZE). This number of bytes immediately prior to the current
// insert_pos are the bytes waiting to be output. If the insert_pos reaches
// the end of the buffer it will wrap around to the beginning (assuming those
// bytes have been output). NOTE: OUTPUT_BUFFER_SIZE can not be larger than
// 255 without changing the type of the variables below (currently defined as
// 8-bit unsigned ints).
#define OUTPUT_BUFFER_SIZE 255
volatile char out_buffer[OUTPUT_BUFFER_SIZE];
volatile uint8_t out_insert_pos;
volatile uint8_t bytes_in_out_buffer;

// Circular buffer to hold incoming characters. Works on same principle
// as output buffer.
#define INPUT_BUFFER_SIZE 16
volatile char input_buffer[INPUT_BUFFER_SIZE];
volatile uint8_t input_insert_pos;
volatile uint8_t bytes_in_input_buffer;
volatile uint8_t input_overrun;

// Variable to keep track of whether incoming characters are to be echoed
// back or not.
static bool do_echo;

static int uart_put_char(char c, FILE *stream)
{
	// Add the character to the buffer for transmission (if there is space
	// to do so). If not we wait until the buffer has space.

	// If the character is linefeed, we output carriage return.
	if (c == '\n')
	{
		uart_put_char('\r', stream);
	}

	// If the buffer is full and interrupts are disabled then we abort -
	// we don't output the character since the buffer will never be
	// emptied if interrupts are disabled. If the buffer is full and
	// interrupts are enabled, then we loop until the buffer has enough
	// space. The bytes_in_buffer variable will get modified by the ISR
	// which extracts bytes from the buffer.
	bool interrupts_enabled = bit_is_set(SREG, SREG_I);
	while (bytes_in_out_buffer >= OUTPUT_BUFFER_SIZE)
	{
		if (!interrupts_enabled)
		{
			return 1;
		}
	}

	// Add the character to the buffer for transmission if there is space
	// to do so. We advance the insert_pos to the next character position.
	// If this is beyond the end of the buffer, we wrap around back to the
	// beginning of the buffer. NOTE: We disable interrupts before
	// modifying the buffer. This prevents the ISR from modifying the
	// buffer at the same time. We reenable them if they were enabled when
	// we entered the function.
	cli();
	out_buffer[out_insert_pos++] = c;
	bytes_in_out_buffer++;
	if (out_insert_pos == OUTPUT_BUFFER_SIZE)
	{
		// Wrap around buffer pointer if necessary.
		out_insert_pos = 0;
	}

	// Reenable interrupts (UDR Empty interrupt may have been disabled) -
	// we ensure it is now enabled so that it will fire and deal with the
	// next character in the buffer.
	UCSR0B |= (1 << UDRIE0);
	if (interrupts_enabled)
	{
		sei();
	}
	return 0;
}

static int uart_get_char(FILE *stream)
{
	// Wait until we've received a character.
	while (bytes_in_input_buffer == 0)
	{
		// Do nothing.
	}

	// Turn interrupts off and remove a character from the input buffer.
	// We reenable interrupts if they were on. The pending character is
	// the one which is byte_in_input_buffer characters before the insert
	// position (taking into account that we may need to wrap around).
	uint8_t interrupts_enabled = bit_is_set(SREG, SREG_I);
	cli();
	char c;
	if (input_insert_pos - bytes_in_input_buffer < 0)
	{
		// Need to wrap around.
		c = input_buffer[input_insert_pos - bytes_in_input_buffer +
			INPUT_BUFFER_SIZE];
	}
	else
	{
		c = input_buffer[input_insert_pos - bytes_in_input_buffer];
	}

	// Decrement our count of bytes in the input buffer.
	bytes_in_input_buffer--;
	if (interrupts_enabled)
	{
		sei();
	}

	// Secretly map the arrows keys to WASD. We essentially replace the
	// last char of the arrow key escape sequences with WASD. This will
	// render them invalid/wrong, but since students aren't expected to
	// handle escape sequences in their code, they would simply see them
	// as WASD. If you're a student reading this, pretend you didn't see
	// it XD. Honestly, you cannot rely on the arrow keys to work like
	// WASD, this is what we call undocumented behaviour.
	static char first = 0;
	static char second = 0;
	if (first == 0x1B && second == '[')
	{
		switch (c)
		{
			case 'A':
				c = 'w';
				break;
			case 'B':
				c = 's';
				break;
			case 'C':
				c = 'd';
				break;
			case 'D':
				c = 'a';
				break;
			default:
				break;
		}
	}
	first = second;
	second = c;

	return c;
}

// File stream which performs I/O using the UART. Used as stdio and stdout.
static FILE serialio = FDEV_SETUP_STREAM(uart_put_char, uart_get_char,
	_FDEV_SETUP_RW);

// Interrupt handler for UART Data Register Empty (i.e., another character
// can be taken from our buffer and written out).
ISR(USART0_UDRE_vect)
{
	// Check if we have data in our buffer.
	if (bytes_in_out_buffer > 0)
	{
		// Yes we do - remove the pending byte and output it via the
		// UART. The pending byte (character) is the one which is
		// bytes_in_buffer characters before the insert_pos (taking
		// into account that we may need to wrap around to the end of
		// the buffer).
		char c;
		if (out_insert_pos - bytes_in_out_buffer < 0)
		{
			// Need to wrap around.
			c = out_buffer[out_insert_pos - bytes_in_out_buffer +
				OUTPUT_BUFFER_SIZE];
		}
		else
		{
			c = out_buffer[out_insert_pos - bytes_in_out_buffer];
		}

		// Decrement our count of the number of bytes in the buffer.
		bytes_in_out_buffer--;

		// Output the character via the UART.
		UDR0 = c;
	}
	else
	{
		// No data in the buffer. We disable the UART Data Register
		// Empty interrupt because otherwise it will trigger again
		// immediately when this ISR exits. The interrupt is reenabled
		// when a character is placed in the buffer.
		UCSR0B &= ~(1 << UDRIE0);
	}
}

// Interrupt handler for UART Receive Complete (i.e., can read a character).
// The character is read and placed in the input buffer.
ISR(USART0_RX_vect)
{
	// Read the character - we ignore the possibility of overrun.
	char c = UDR0;

	if (do_echo && bytes_in_out_buffer < OUTPUT_BUFFER_SIZE)
	{
		// If echoing is enabled and there is output buffer space,
		// echo the received character back to the UART. If there
		// is no output buffer space, characters will be lost.
		uart_put_char(c, 0);
	}

	// Check if we have space in our buffer. If not, set the overrun flag
	// and throw away the character. We never clear the overrun flag -
	// it's up to the programmer to check/clear this flag if desired.
	if (bytes_in_input_buffer >= INPUT_BUFFER_SIZE)
	{
		input_overrun = 1;
	}
	else
	{
		// If the character is carriage return, turn it into linefeed.
		if (c == '\r')
		{
			c = '\n';
		}

		// There is room in the input buffer.
		input_buffer[input_insert_pos++] = c;
		bytes_in_input_buffer++;
		if (input_insert_pos == INPUT_BUFFER_SIZE)
		{
			// Wrap around buffer pointer if necessary.
			input_insert_pos = 0;
		}
	}
}

void init_serial_stdio(long baudrate, bool echo)
{
	// Initialise our buffers.
	out_insert_pos = 0;
	bytes_in_out_buffer = 0;
	input_insert_pos = 0;
	bytes_in_input_buffer = 0;
	input_overrun = 0;

	// Record whether we're going to echo characters or not.
	do_echo = echo;

	// Configure the baud rate. This differs from the datasheet formula so
	// that we get rounding to the nearest integer while using integer
	// division (which truncates).
	UBRR0 = (uint16_t)((((SYSCLK / (8 * baudrate)) + 1) / 2) - 1);

	// Enable transmission and receiving via UART. We don't enable the UDR
	// empty interrupt here (we wait until we've got a character to
	// transmit). NOTE: Interrupts must be enabled globally for this
	// module to work, but we do not do this here.
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// Enable receive complete interrupt.
	UCSR0B |= (1 << RXCIE0);

	// Set up our stream so the get and put functions are used to
	// read/write characters via the serial port when we use stdio
	// functions.
	stdout = &serialio;
	stdin = &serialio;
}

bool serial_input_available(void)
{
	return bytes_in_input_buffer != 0;
}

void clear_serial_input_buffer(void)
{
	// Just adjust our buffer data so it looks empty.
	input_insert_pos = 0;
	bytes_in_input_buffer = 0;
}
