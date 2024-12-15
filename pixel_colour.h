/*
 * pixel_colour.h
 *
 * Author: Peter Sutton
 * 
 * Definitions for LED matrix colours.
 */ 

#ifndef PIXEL_COLOUR_H_
#define PIXEL_COLOUR_H_

#include <stdint.h>

// Each PixelColour is an 8-bit number - 4 bits of green in the high bits,
// 4 bits of red in the low bits.
typedef uint8_t PixelColour;

// You may modify the following colour definitions if you are colour blind,
// however you must document your modifications in your Feature Summary,
// otherwise deductions may be made if the colours appear wrong to markers
// with normal colour vision.

#define COLOUR_BLACK       	(0x00)
#define COLOUR_RED         	(0x0F)
#define COLOUR_LIGHT_GREEN 	(0x11)
#define COLOUR_GREEN       	(0xF0)
#define COLOUR_DARK_GREEN  	(0x10)
#define COLOUR_LIGHT_YELLOW	(0x35)
#define COLOUR_YELLOW      	(0xFF)
#define COLOUR_LIGHT_ORANGE	(0x13)
#define COLOUR_ORANGE      	(0x3C)

#endif /* PIXEL_COLOUR_H_ */
