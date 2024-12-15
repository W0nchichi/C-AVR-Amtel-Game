/*
 * game.c
 *
 * Authors: Jarrod Bennett, Cody Burnett, Bradley Stone, Yufeng Gao
 * Modified by: <YOUR NAME HERE>
 *
 * Game logic and state handler.
 */ 

#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "terminalio.h"
#include "Beeper.h"

// ========================== NOTE ABOUT MODULARITY ==========================

// The functions and global variables defined with the static keyword can
// only be accessed by this source file. If you wish to access them in
// another C file, you can remove the static keyword, and define them with
// the extern keyword in the other C file (or a header file included by the
// other C file). While not assessed, it is suggested that you develop the
// project with modularity in mind. Exposing internal variables and functions
// to other .C files reduces modularity.


// ============================ GLOBAL VARIABLES =============================

// The game board, which is dynamically constructed by initialise_game() and
// updated throughout the game. The 0th element of this array represents the
// bottom row, and the 7th element of this array represents the top row.

// The location of the player.

// A flag for keeping track of whether the player is currently visible.
static bool player_visible;


//step count for player
//seven seg numbers
//uint8_t seven_seg_data[10] = {63,6,91,79,102,109,125,7,127,111};
//volatile uint8_t seven_seg_cc = 0;

typedef struct {
	int8_t delta_row;
	int8_t delta_col;
	bool box_moved;
} movement;

movement moves_stack[6];
movement redo_stack[6];

// Variables to track the current size of the stacks
uint8_t moves_stack_size = 0;
uint8_t redo_stack_size = 0;

uint8_t undo_index = 0;

//to flash (tf) red
bool tf = true;


// ========================== GAME LOGIC FUNCTIONS ===========================

// This function paints a square based on the object(s) currently on it.
static void paint_square(uint8_t row, uint8_t col){
	switch (board[row][col] & OBJECT_MASK)
	{
		case ROOM: ledmatrix_update_pixel(row, col, COLOUR_BLACK); break;
		case WALL: ledmatrix_update_pixel(row, col, COLOUR_WALL); break;
		case BOX: ledmatrix_update_pixel(row, col, COLOUR_BOX); break;
		case TARGET: ledmatrix_update_pixel(row, col, COLOUR_TARGET); break;
		case BOX | TARGET: ledmatrix_update_pixel(row, col, COLOUR_DONE); break;
		default: break;
	}
	// Update terminal view to match LED matrix exactly.
	// Make sure to invert terminal rows relative to LED matrix.
	// All colours standardised --> let bow = BG_WHITE.
	move_terminal_cursor((MATRIX_NUM_ROWS - row) + 5, col * 2 + 1);
	uint8_t object = board[row][col] & OBJECT_MASK;
	switch (object) {
		case ROOM: set_display_attribute(BG_BLACK); break;
		case WALL: set_display_attribute(BG_YELLOW); break;
		case BOX: set_display_attribute(BG_WHITE); break;
		case TARGET: set_display_attribute(BG_RED); break;
		case BOX | TARGET: set_display_attribute(BG_GREEN); break;
		default: break;
	}
	// Print two spaces for less squash.
	printf("  ");
	normal_display_mode();
}

// This function initialises the global variables used to store the game
// state, and renders the initial game display.
void initialise_game(uint8_t level)
{
	// Short definitions of game objects used temporarily for constructing
	// an easier-to-visualise game layout.
	#define _	(ROOM)
	#define W	(WALL)
	#define T	(TARGET)
	#define B	(BOX)

	// The starting layout of level 1. In this array, the top row is the
	// 0th row, and the bottom row is the 7th row. This makes it visually
	// identical to how the pixels are oriented on the LED matrix, however
	// the LED matrix treats row 0 as the bottom row and row 7 as the top
	// row.
	static const uint8_t lv1_layout[MATRIX_NUM_ROWS][MATRIX_NUM_COLUMNS] =
	{
		{ _, W, _, W, W, W, _, W, W, W, _, _, W, W, W, W },
		{ _, W, T, W, _, _, W, T, _, B, _, _, _, _, T, W },
		{ _, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _ },
		{ W, _, B, _, _, _, _, W, _, _, B, _, _, B, _, W },
		{ W, _, _, _, W, _, B, _, _, _, _, _, _, _, _, _ },
		{ _, _, _, _, _, _, T, _, _, _, _, _, _, _, _, _ },
		{ _, _, _, W, W, W, W, W, W, T, _, _, _, _, _, W },
		{ W, W, _, _, _, _, _, _, W, W, _, _, W, W, W, W }
	};
	static const uint8_t lv2_layout[MATRIX_NUM_ROWS][MATRIX_NUM_COLUMNS] =
	{
		{ _, _, W, W, W, W, _, _, W, W, _, _, _, _, _, W },
		{ _, _, W, _, _, W, _, W, W, _, _, _, _, B, _, _ },
		{ _, _, W, _, B, W, W, W, _, _, T, W, _, T, W, W },
		{ _, _, W, _, _, _, _, T, _, _, B, W, W, W, _, _ },
		{ W, W, W, W, _, W, _, _, _, _, _, W, _, W, W, _ },
		{ W, T, B, _, _, _, _, B, _, _, _, W, W, _, W, W },
		{ W, _, _, _, T, _, _, _, _, _, _, B, T, _, _, _ },
		{ W, W, W, W, W, W, W, W, W, W, W, W, W, W, W, W }
	};


	// Undefine the short game object names defined above, so that you
	// cannot use use them in your own code. Use of single-letter names/
	// constants is never a good idea.
	#undef _
	#undef W
	#undef T
	#undef B
	if(level == 1){
		// Set the initial player location (for level 1).
		player_row = 5;
		player_col = 2;

		// Make the player icon initially invisible.
		player_visible = false;

		// Copy the starting layout (level 1 map) to the board array, and flip
		// all the rows.
		for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)	{
			for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
				board[MATRIX_NUM_ROWS - 1 - row][col] =
				lv1_layout[row][col];
			}
		}
		// Draw the game board (map).
		for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++) {
			for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
				paint_square(row, col);
			}
		}
	} else if(level == 2) {
		// Set the initial player location (for level 1).
		player_row = 6;
		player_col = 15;


		// Make the player icon initially invisible.
		player_visible = false;

		// Copy the starting layout (level 1 map) to the board array, and flip
		// all the rows.
		for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)	{
			for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
				board[MATRIX_NUM_ROWS - 1 - row][col] =
				lv2_layout[row][col];
			}
		}
		// Draw the game board (map).
		for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++) {
			for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
				paint_square(row, col);
			}
		}
	}
}

// This function flashes the player icon. If the icon is currently visible, it
// is set to not visible and removed from the display. If the player icon is
// currently not visible, it is set to visible and rendered on the display.
// The static global variable "player_visible" indicates whether the player
// icon is currently visible.

void flash_player(void)
{
	player_visible = !player_visible;
	if (player_visible)
	{
		// Update the LED matrix for the player position
		ledmatrix_update_pixel(player_row, player_col, COLOUR_PLAYER);

		// Move the terminal cursor to the player's position
		move_terminal_cursor((MATRIX_NUM_ROWS - player_row) + 5, player_col * 2 + 1);

		// Set the display attribute (BG_CYAN) and print spaces with cyan background
		set_display_attribute(BG_CYAN);
		printf("  ");  // This will appear with cyan background

		// Reset the display mode back to normal after printing
		normal_display_mode();
	}
	else
	{
		// If the player is not visible, repaint the square to its original state
		paint_square(player_row, player_col);
	}
}

void redrawing_from_loading(void){
	for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++)	{
		for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
			paint_square(row, col);
		}
	}
}


//UNDO MOVES
//As an array
//NEED TO FIX TARGER|BOX SPAWN
//NEED TO IMPLEMENT LEDs
uint8_t undo_moves(void) {
	if (moves_stack_size == 0) {
		return 0;
	}

	movement last_move = moves_stack[--moves_stack_size];

	if (redo_stack_size < 6) {
		redo_stack[redo_stack_size++] = last_move;
	}
	
	int8_t new_box_row = player_row;
	int8_t new_box_col = player_col;
	
	int8_t new_player_row = (player_row - last_move.delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
	int8_t new_player_col = (player_col - last_move.delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
	
	player_col = new_player_col;
	player_row = new_player_row;
	
	
	if (last_move.box_moved) {
		int8_t new_target_row = (new_box_row + last_move.delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
		int8_t new_target_col = (new_box_col + last_move.delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
		
		board[new_box_row][new_box_col] |= BOX;
		board[new_target_row][new_target_col] &= ~BOX;

		
		paint_square(new_target_row, new_target_col);
	}
	
	paint_square(new_box_row, new_box_col);
	if (last_move.delta_row != 0 && last_move.delta_col != 0){
		return 2;
	}
	return 1;
}

uint8_t redo_moves(void) {
	if (redo_stack_size == 0) {
		return 0; 
	}

	movement last_move = redo_stack[--redo_stack_size];  // Get the last move to redo

	if (moves_stack_size < 6) {
		moves_stack[moves_stack_size++] = last_move;
	}

	int8_t new_player_row = (player_row + last_move.delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
	int8_t new_player_col = (player_col + last_move.delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
	paint_square(player_row, player_col);
	
	player_row = new_player_row;           
	player_col = new_player_col;
	

	// If a box was moved, redo the box movement as well
	if (last_move.box_moved) {
		int8_t new_box_row = (player_row + last_move.delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
		int8_t new_box_col = (player_col + last_move.delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;

		// Move the box to the new position
		board[new_box_row][new_box_col] |= BOX;
		board[player_row][player_col] &= ~BOX;

		paint_square(new_box_row, new_box_col); 
	}

	// Return whether it was a diagonal move or not
	if (last_move.delta_row != 0 && last_move.delta_col != 0) {
		return 2;  
	}
	return 1; 
}

void update_player_location(int8_t delta_row, int8_t delta_col)
{
	//update cols and rows, use a modulo to find if it = 0, or was already 0
	player_col = (player_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
	player_row = (player_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
	
	player_visible = true;
	ledmatrix_update_pixel(player_row, player_col, COLOUR_PLAYER);
	move_terminal_cursor((MATRIX_NUM_ROWS - player_row) + 5, player_col * 2 + 1);

	// Set the display attribute (BG_CYAN) and print spaces with cyan background
	set_display_attribute(BG_CYAN);
	printf("  ");  // This will appear with cyan background

	// Reset the display mode back to normal after printing
	normal_display_mode();	
}

bool is_valid_move(int8_t delta_row, int8_t delta_col){
	// Calculate the new potential position using wrap-around logic
	int8_t new_row = (player_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
	int8_t new_col = (player_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
	
	if(board[new_row][new_col] == WALL){
		return false;
	}
	if (delta_row != 0 && delta_col != 0) {  // This is a diagonal movement
		int8_t adj_row = (player_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
		int8_t adj_col = (player_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;

		// Check if there's a wall either vertically or horizontally adjacent
		if (board[adj_row][player_col] == WALL && board[player_row][adj_col] == WALL) {
			return false;
		}
	}
	return true;
}
void hit_wall_message(){
	int x = rand() % 3;
	if (x == 0){
		printf("the player has hit a wall :'(\n");
	} else if (x == 1){
		printf("Bam! There's a wall in your way fool!\n");
	} else {
		printf("Thud! The wall stopped your path.\n");
	}
}

void move_box_message(){
	int x = rand() % 3;
	if (x == 0){
		printf("omg you can't move boxes there!!!\n");
		} else if (x == 1){
		printf("CRASH! Great, now you've damaged the box :/\n");
		} else {
		printf("The box's natural predator, the wall - stands in your way\n");
	}
}

void move_box_diagonal_message(){
	int x = rand() % 3;
	if (x == 0){
		printf("NO MOVING BOXES DIAGONALLY!!!! >:(\n");
		} else if (x == 1){
		printf("There's no way you seriously just tried to move a box... diagonally?\n");
		} else {
		printf("reality collapses around you as you try to move the box diagonally\n");
	}
}

void flash_target_animation(void){
	tf = !tf;
	for (int i = 0; i < MATRIX_NUM_ROWS; i++) {
		for(int j = 0; j < MATRIX_NUM_COLUMNS; j++) {
			if(board[i][j] & TARGET) {
				if(board[i][j] & BOX){
					//matrix
					ledmatrix_update_pixel(i, j, COLOUR_DONE);
					//terminal
					move_terminal_cursor((MATRIX_NUM_ROWS - i) + 5, j * 2 + 1);
					set_display_attribute(BG_GREEN);
					printf("  ");
					normal_display_mode();
				} else {
					if(tf){
						ledmatrix_update_pixel(i, j, COLOUR_TARGET);
						move_terminal_cursor((MATRIX_NUM_ROWS - i) + 5, j * 2 + 1);
						set_display_attribute(BG_RED);
						printf("  ");
						normal_display_mode();
					} else {
						ledmatrix_update_pixel(i, j, COLOUR_BLACK);
						move_terminal_cursor((MATRIX_NUM_ROWS - i) + 5, j * 2 + 1);
						set_display_attribute(BG_BLACK);
						printf("  ");
						normal_display_mode();
					}
				}
			}
		}
	} 
}

bool box_push(int8_t delta_row, int8_t delta_col){
	int8_t new_row = (player_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
	int8_t new_col = (player_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
	
	if (delta_row != 0 && delta_col != 0) {
		// Diagonal movement is not allowed for boxes
		move_box_diagonal_message();
		return false;
	}
	
	if (board[new_row][new_col] == BOX) {
		// Calculate the position behind the box
		int8_t next_row = (new_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
		int8_t next_col = (new_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
		
		// Check if the space behind the box is a wall or another box
		if (board[next_row][next_col] == WALL || board[next_row][next_col] == BOX) {
		// Box cannot be pushed into a wall, return false
			return false;
		}
		if (board[next_row][next_col] == TARGET){
			board[next_row][next_col] = BOX | TARGET;
			board[player_row][player_col] = ROOM; 
			board[new_row][new_col] = ROOM;
			paint_square(next_row, next_col);
			return true;
		} 
		if ((board[next_row][next_col] & (BOX | TARGET)) == (BOX | TARGET)){
			return false;
		}
		else {
		board[next_row][next_col] = BOX; // Move the box
		board[new_row][new_col] = ROOM;  // Clear the previous box position, don't paint, it will be overridden by char
		paint_square(next_row, next_col);
		return true; // Box was successfully pushed
		}
	}
	return false;
}

bool target_box_push(int8_t delta_row, int8_t delta_col) {
	// Calculate the new position of the box the player is trying to push (currently on BOX | TARGET)
	int8_t new_row = (player_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
	int8_t new_col = (player_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;
	
	if (delta_row != 0 && delta_col != 0) {
		// Diagonal movement is not allowed for boxes
		move_box_diagonal_message();
		return false;
	}

	if ((board[new_row][new_col] & (BOX | TARGET)) == (BOX | TARGET)) {
		// Calculate the position behind the box
		int8_t next_row = (new_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
		int8_t next_col = (new_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;

		// Check if the space behind the box is a wall or another box
		if (board[next_row][next_col] == WALL || board[next_row][next_col] == BOX) {
			// Box cannot be pushed into a wall or another box
			return false;
		}

		// Case where the box is being pushed onto a target
		if (board[next_row][next_col] == TARGET) {
			board[next_row][next_col] = BOX | TARGET;  // Set the box on the new target
			board[player_row][player_col] = ROOM;      // Clear the player's previous position
			board[new_row][new_col] = TARGET;          // Restore the target where the box was
			paint_square(next_row, next_col);          // Paint the new position (box on target)
			paint_square(new_row, new_col);            // Paint the restored target
			return true;
		}

		// Move the box to an empty space
		board[next_row][next_col] = BOX;               // Move the box
		board[new_row][new_col] = TARGET;              // Restore the target where the box was
		board[player_row][player_col] = ROOM;          // Clear the player's previous position
		paint_square(next_row, next_col);              // Paint the new box position
		paint_square(new_row, new_col);                // Paint the restored target
		return true; // Box was successfully pushed off the target
	}
	return false; // The box was not on a target or could not be moved
}


// This function handles player movements.
bool move_player(int8_t delta_row, int8_t delta_col) {
    bool box_moved = false;
    paint_square(player_row, player_col);
    move_terminal_cursor(1,1);
    clear_to_end_of_line();
    
    int8_t new_row = (player_row + delta_row + MATRIX_NUM_ROWS) % MATRIX_NUM_ROWS;
    int8_t new_col = (player_col + delta_col + MATRIX_NUM_COLUMNS) % MATRIX_NUM_COLUMNS;

    // Check if the desired position is a box
    if (board[new_row][new_col] == BOX) {
        if (!box_push(delta_row, delta_col)) {
			if (!(player_col != 0 && player_row != 0)){
				move_box_message();
			}
            return false;
        }
		box_moved = true;
    } else if ((board[new_row][new_col] & (BOX | TARGET)) == (BOX | TARGET)) {
        if (!target_box_push(delta_row, delta_col)) {
            if (!(player_col != 0 && player_row != 0)){
	            move_box_message();
            }
            return false;
        }
		box_moved = true;
    }

    // Check if the move is valid (not a wall)
    if (is_valid_move(delta_row, delta_col)) {
		paint_square(player_row, player_col);
        update_player_location(delta_row, delta_col);

        // Store the new move in the undo stack
        // Store the new move in the undo stack
        movement new_move;
        new_move.delta_row = delta_row;
        new_move.delta_col = delta_col;
        new_move.box_moved = box_moved;

        // Push the move into the moves_stack
        if (moves_stack_size < 6) {  // Ensure that the stack doesn't overflow
	        moves_stack[moves_stack_size] = new_move;
	        moves_stack_size++;
	        } else {
	        // You could discard the oldest move here if desired:
	        for (int i = 1; i < 6; i++) {
		        moves_stack[i - 1] = moves_stack[i];
	        }
	        moves_stack[5] = new_move;
        }
		redo_stack_size = 0;


        return true;
    } else {
        hit_wall_message();
        return false;
    }
}

// This function checks if the game is over (i.e., the level is solved), and
// returns true iff (if and only if) the game is over.
bool is_game_over(void) {
	for (uint8_t row = 0; row < MATRIX_NUM_ROWS; row++) {
		for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
			if ((board[row][col] & TARGET) && !(board[row][col] & BOX)) {
				return false;
			}
		}
	}
	return true;
}