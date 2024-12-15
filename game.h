/*
 * game.h
 *
 * Authors: Jarrod Bennett, Cody Burnett, Bradley Stone, Yufeng Gao
 * Modified by: <YOUR NAME HERE>
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */

#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>
#include <stdbool.h>

// Object definitions.
#define ROOM       	(0U << 0)
#define WALL       	(1U << 0)
#define BOX        	(1U << 1)
#define TARGET     	(1U << 2)
#define OBJECT_MASK	(ROOM | WALL | BOX | TARGET)

// Colour definitions.
#define COLOUR_PLAYER	(COLOUR_DARK_GREEN)
#define COLOUR_WALL  	(COLOUR_YELLOW)
#define COLOUR_BOX   	(COLOUR_ORANGE)
#define COLOUR_TARGET	(COLOUR_RED)
#define COLOUR_DONE  	(COLOUR_GREEN)

/// <summary>
/// Initialises the game.
/// </summary>
void initialise_game(void);

/// <summary>
/// Moves the player based on row and column deltas.
/// </summary>
/// <param name="delta_row">The row delta.</param>
/// <param name="delta_col">The column delta.</param>
void move_player(int8_t delta_row, int8_t delta_col);

/// <summary>
/// Detects whether the game is over (i.e., current level solved).
/// </summary>
/// <returns>Whether the game is over.</returns>
bool is_game_over(void);

/// <summary>
/// Flashes the player icon.
/// </summary>
void flash_player(void);

#endif /* GAME_H_ */
