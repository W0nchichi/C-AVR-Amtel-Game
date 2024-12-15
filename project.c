/*
 * project.c
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett,
 *          Bradley Stone, Yufeng Gao
 * Modified by: <YOUR NAME HERE>
 *
 * Main project event loop and entry point.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "startscrn.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"
#include "FileSaver.h"

// Define Deadzone Constants
#define DEADZONE_LOW  340
#define DEADZONE_HIGH 682

//Define LED stuff
#define LED_PORT PORTA
#define LED_DDR  DDRA
#define LED_PINS 0x00  // Binary: 11111100 (A2 to A7)


// Function prototypes - these are defined below (after main()) in the order
// given here.
void initialise_hardware(void);
void start_screen(void);
void new_game(uint8_t level, bool saved);
void play_game(void);
void handle_game_over(void);

bool game_over_flag = false;
static uint16_t steps = 0;
static uint32_t level_time = 0;
static uint8_t level = 1;
uint8_t x_or_y = 0;
uint16_t value;
bool music_paused;
volatile uint8_t undo_count = 0; // Tracks number of undo actions
bool is_game_saved = false;


/////////////////////////////// main //////////////////////////////////
int main(void)
{
	// Setup hardware and callbacks. This will turn on interrupts.
	initialise_hardware();

	// Show the start screen. Returns when the player starts the game.
	

	// Loop forever and continuously play the game.
	while (1)
	{
		start_screen();
		handle_game_over();
	}
}


void initialise_hardware(void)
{
	init_ledmatrix();
	init_buttons();
	init_serial_stdio(19200, false);
	init_timer0();
	init_timer1();
	init_timer2();
	
	//SSD ports
	DDRC = 0xFF; 
	DDRD |= (1 << 2);
	
	DDRD |= (1 << 4);
	
	//Joysick setup
	ADMUX = (1<<REFS0);
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1);
	
	//light up pins (might conflict with joystick)
	LED_DDR |= LED_PINS;    // Set A2-A7 as output
	LED_PORT |= LED_PINS;   // Initialize LEDs to OFF

	// Turn on global interrupts.
	sei();
}

void timer_seconds(uint32_t start_time) {
	// since 1s = 1000ms, reset then.
	level_time+=1;
	set_display_attribute(BG_BLACK);  // Black background

	// Move cursor to fixed position + update timer display.
	move_terminal_cursor(3, 1);
	set_display_attribute(BG_BLACK);
	printf_P(PSTR("Time spent: %lu seconds   "), level_time);
}

static uint8_t seven_seg_cc = 0;

void update_ssd_steps(uint32_t step_count){
	
	uint8_t seven_seg_data[10] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111};
	
	seven_seg_cc ^= 1;
	if (seven_seg_cc == 0){
		PORTC = seven_seg_data[(step_count / 10) % 10];
		PORTD |= (1 << 2);
	}
	else{
		PORTC = seven_seg_data[step_count % 10];
		PORTD &= ~(1 << 2); 
	}
}


void update_undo_leds(void) {
	// Turn off all undo LEDs
	LED_PORT &= LED_PINS; // Common Anode: 1 turns off LEDs
	
	// Light up 'undo_count' LEDs
	for(uint8_t i = 0; i < undo_count && i < 6; i++) {
		LED_PORT |= (1 << (i + 2)); 
	}
}


void start_screen(void)
{
	end_music();
	// Hide terminal cursor and set display mode to default.
	hide_cursor();
	normal_display_mode();
	
	uint32_t last_sound_time = get_current_time();
	uint8_t music_stage = 0;
	uint32_t current_time = get_current_time();
	
	bool game_to_load = false;
	
	
	if (eeprom_read_word((uint16_t*)0) == EEPROM_SIG_VALUE)
	{
		game_to_load = true;
	}
	

	// Clear terminal screen and output the title ASCII art.
	clear_terminal();
	display_terminal_title(3, 5);
	move_terminal_cursor(11, 5);
	// Change this to your name and student number. Remember to remove the
	// chevrons - "<" and ">"!
	printf_P(PSTR("CSSE2010/7201 Project by Luca Rovera - 48904838"));

	// Setup the start screen on the LED matrix.
	setup_start_screen();

	// Clear button presses registered as the result of powering on the
	// I/O board. This is just to work around a minor limitation of the
	// hardware, and is only done here to ensure that the start screen is
	// not skipped when you power cycle the I/O board.
	clear_button_presses();
	
	if (game_to_load) {
		move_terminal_cursor(13, 15);
		printf_P(PSTR("Press 'R' to load the saved game"));
		move_terminal_cursor(14, 15);
		printf_P(PSTR("Press 'D' to delete the saved game"));
	}

	// Wait until a button is pushed, or 's'/'S' is entered.
	while (1)
	{
		music_stage = 0;
		// Check for button presses. If any button is pressed, exit
		// the start screen by breaking out of this infinite loop.
		if (button_pushed() != NO_BUTTON_PUSHED)
		{
			return 1;
		}

		// No button was pressed, check if we have terminal inputs.
		if (serial_input_available())
		{
			// Terminal input is available, get the character.
			int serial_input = fgetc(stdin);

			// If the input is 's'/'S', exit the start screen by
			// breaking out of this loop.
			if (serial_input == 's' || serial_input == 'S')
			{
				break;

			} else if (serial_input == '2') {
				level = 2;
				break;
			}
			if (toupper(serial_input) == 'R' && game_to_load)
			{
				load_saved_game();
			}
			if (toupper(serial_input) == 'D' && game_to_load)
			{
				reset_game_save();
				move_terminal_cursor(13, 15);
				clear_to_end_of_line();
				move_terminal_cursor(14, 15);
				clear_to_end_of_line();
			}
			
		}

		// No button presses and no 's'/'S' typed into the terminal,
		// we will loop back and do the checks again. We also update
		// the start screen animation on the LED matrix here.
		update_start_screen();
	}
	
	new_game(level, false);
	play_game();
}

void new_game(uint8_t level, bool saved)
{
	
	// Clear the serial terminal.
	hide_cursor();
	clear_terminal();
	
	if (!saved)
	{
		level_time = 0;
		steps = 0;
		initialise_game(level);
	}
	else
	{
		redrawing_from_loading();
	}


	// Clear all button presses and serial inputs, so that potentially
	// buffered inputs aren't going to make it to the new game.
	clear_button_presses();
	clear_serial_input_buffer();
}

uint16_t read_adc() {
	// Set the ADC mux to choose ADC0 if x_or_y is 0, ADC1 if x_or_y is 1
	if(x_or_y == 0) {
		ADMUX &= ~1;
		} else {
		ADMUX |= 1;
	}
	// Start the ADC conversion
	ADCSRA |= (1<<ADSC);
	
	while(ADCSRA & (1<<ADSC)) {
		; /* Wait until conversion finished */
	}
	value = ADC; // read the value
	// Next time through the loop, do the other direction
	x_or_y ^= 1;
	return ADC;
}

void save_current_game(){
    uint8_t current_level = level; // Assuming 'level' is a global variable
    save_game(current_level, steps, level_time);
}

// Example function to load a saved game
void load_saved_game(){
    uint8_t saved_level;
	uint16_t numsteps;
	uint32_t lvltime;
	bool saved = load_game(&saved_level, &numsteps, &lvltime);
	
    if(saved)
	{
		delete_game();
        level = saved_level;
		steps = numsteps;
		level_time = lvltime;
        // Initialize the game with the loaded level
        new_game(level, true);
        play_game();
    }
	
    else{
        // No saved game found; start a new game or handle accordingly
        level = 1;
        new_game(level, false);
        play_game();
    }
}

void reset_game_save(){
    delete_game();
    level = 1;
}

void play_game(void) {
	uint32_t last_timer_time = get_current_time();
	uint32_t last_flash_time = get_current_time();
	uint32_t start_time = get_current_time();
	uint32_t last_sound_time = get_current_time();
	uint32_t last_step_time = get_current_time();
	uint32_t last_anim_time = get_current_time();
	uint8_t music_stage = 0;
	//for ssd
	uint32_t last_ssd_time = get_current_time();
	//for pausing
	bool paused = false;
	//for joystick
	const uint16_t THRESHOLD_HIGH = DEADZONE_HIGH;
	const uint16_t THRESHOLD_LOW  = DEADZONE_LOW;
	uint32_t last_joystick_time = get_current_time();

	

	while (!is_game_over()) {		
		uint32_t current_time = get_current_time();
		if(current_time >= last_ssd_time + 10){
			update_ssd_steps(steps);
			last_ssd_time = current_time;
		}
		
		ButtonState btn = button_pushed();
		int serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		if (toupper(serial_input) == 'P') {
			paused = !paused;			
			if (paused) {
				end_music();
				normal_display_mode();
				move_terminal_cursor(1,1);
				printf_P(PSTR("paused, Press 'p' to resume."));
				set_display_attribute(FG_BLACK);
			} else {
				move_terminal_cursor(1,1);
				clear_to_end_of_line();
			}
		}
		
		if (paused)
		{
			if (current_time >= last_ssd_time + 10)
			{
				update_ssd_steps(steps);
				last_ssd_time = current_time;
			}
			continue;
		}
		
		if (toupper(serial_input) == 'Z') {
			uint8_t result = undo_moves();
			//result = 0 is didn't work
			//result = 1 is worked but not diagonal
			//result = 2 is worked and was diagonal
			 if(result > 0) {
				 if(result == 2) {
					 steps--;
				 }
				 steps--;
				 if(undo_count > 0) {
					 undo_count--;
					 update_undo_leds();
				 }
			 }
			 
		} 
		if (toupper(serial_input) == 'Y') {
			uint8_t result = redo_moves();
			if (result > 0){
				steps = steps + 1;
				if (undo_count < 6){
					undo_count++;
					update_undo_leds();
				}
				if (result == 2){
					steps = steps + 1;
				}
				
			}
		}
		if(toupper(serial_input) == 'Q'){
			music_paused = !music_paused;
			music_stage = 0;
			end_music();
		}
		if (toupper(serial_input) == 'X'){
			move_terminal_cursor(0,0);
			clear_to_end_of_line();
			printf("Y to save");
			while(1){
				if (serial_input_available()) {
					serial_input = fgetc(stdin);
				}
				if(toupper(serial_input) == 'Y'){
					save_current_game();
					clear_terminal();
					start_screen();
					is_game_saved = true;
				}
				if(toupper(serial_input) == 'N'){
					clear_terminal();
					start_screen();
				}
				
			}
		}
		
		bool valid_move_made = false;
		
		//putting joystick controls here:
		int8_t move_x = 0;
		int8_t move_y = 0;
		//added a 200ms delay or something otherwise it's SPEEEEEEED
		if(current_time >= last_joystick_time + 200) {
			// Read X from ADC0
			uint16_t x_value = read_adc(); // A0
			// Read Y from ADC1
			uint16_t y_value = read_adc(); // A1S
						
			// Control player based on X-axis
			if (x_value > THRESHOLD_HIGH) {
				move_x = 1;
				
			}
			else if (x_value < THRESHOLD_LOW) {
				move_x = -1;
			}

			// Control player based on Y-axis
			if (y_value > THRESHOLD_HIGH) {
				move_y = 1;
			}
			else if (y_value < THRESHOLD_LOW) {
				move_y = -1;
			}
			last_joystick_time = current_time;
		}
		//end of joystick controls		
		if (serial_input == 'D' || serial_input == 'd') {
			move_x = 1;
		}
		else if (serial_input == 'A' || serial_input == 'a') {
			move_x = -1;
		}

		if (serial_input == 'W' || serial_input == 'w') {
			move_y = 1;
		}
		else if (serial_input == 'S' || serial_input == 's') {
			move_y = -1;
		}

		// Button controls
		if (btn == BUTTON0_PUSHED){
			move_x = 1;
		}
		else if (btn == BUTTON1_PUSHED) {
			move_y = -1;
		}
		else if(btn == BUTTON2_PUSHED ) {
			move_y = 1;
		}
		else if (btn == BUTTON3_PUSHED ){
			move_x = -1;
		}
		
		 if (move_x != 0 || move_y != 0) 
		 {
			 bool result = move_player(move_y, move_x);
			 if (result) {
				 last_flash_time = current_time;
				 valid_move_made = true;
				 if (move_x + move_y != 1 && move_x + move_y != -1) {
					 steps ++;
					 music_stage = 0;
				 }
				 if(undo_count < 6) {
					 undo_count++;
					 update_undo_leds();
				 }
				 end_music();
				 if (!music_paused && current_time > last_step_time + 20){
					 make_sound(880);
					 last_step_time = current_time;
				 }
				 steps++;
				 last_flash_time = current_time;
			 } else {
				 end_music();
				 if (!music_paused && current_time > last_step_time + 20){
					 make_sound(20);
					 last_step_time = current_time;
				 }
			 }
		 }
		 
		 // oh 200 ms lol

		if (current_time >= last_flash_time + 200) {
			flash_player();
			last_flash_time = current_time;
		}
		
		if (current_time >= last_timer_time + 1000) {
			timer_seconds(start_time);
			last_timer_time = current_time;
		}
		if(current_time > last_anim_time + 500){
			flash_target_animation();
			last_anim_time = current_time;
		}
		if (music_stage == 0 && current_time > last_sound_time + 250 && !music_paused){
			make_sound(110);
			music_stage = 1;
			last_sound_time = current_time;
		}
		if (music_stage == 1 && current_time > last_sound_time + 250 && !music_paused){
			make_sound(139);
			music_stage = 2;
			last_sound_time = current_time;
		}
		if  (music_stage == 2 && current_time > last_sound_time + 250){
			make_sound(220);
			music_stage = 3;
			last_sound_time = current_time;
		}
		if  (music_stage == 3 && current_time > last_sound_time + 250){
			make_sound(247);
			music_stage = 4;
			last_sound_time = current_time;
		}
		if  (music_stage == 4 && current_time > last_sound_time + 250){
			make_sound(277);
			music_stage = 5;
			last_sound_time = current_time;
		}
		if  (music_stage == 5 && current_time > last_sound_time + 250){
			make_sound(247);
			music_stage = 0;
			last_sound_time = current_time;
		}
	}
	game_over_flag = true;
	update_ssd_steps(steps);
}
//

uint32_t calculate_score(uint16_t steps, uint32_t time) {
	uint32_t step_score;
	uint32_t time_score;
	
	// Calculate step_score
	if (steps > 200) {
		step_score = 0;
		} else {
		step_score = (200 - steps) * 20;
	}

	// Calculate time_score
	if (level_time > 1200) {
		time_score = 0;
		} else {
		time_score = 1200 - level_time;
	}

	// Calculate the final score using uint32_t to prevent overflow
	uint32_t score = time_score + step_score;
	return score;
}


void display_level_complete_screen(uint32_t score)
{
	// Clear the terminal screen
	clear_terminal();
	set_display_attribute(BG_BLACK);

	move_terminal_cursor(10, 0);
	printf("+====================================+\n");

	move_terminal_cursor(11, 0);
	printf("|           LEVEL COMPLETE           |\n");

	move_terminal_cursor(12, 0);
	printf("+====================================+\n");

	move_terminal_cursor(13, 0);
	printf("|   Time: %lus                        |\n", level_time);

	move_terminal_cursor(14, 0);
	printf("|   Steps: %lu                        |\n", steps);  // Display steps directly

	move_terminal_cursor(15, 0);
	printf("|   Score: %lu                      |\n", score);

	move_terminal_cursor(16, 0);
	printf("+====================================+\n");

	move_terminal_cursor(17, 0);
	printf("|  [R] Restart | [N] Next | [E] Exit |\n");

	move_terminal_cursor(18, 0);
	printf("+====================================+\n");
}

void handle_game_over(void)
{	
	end_music();
	uint32_t score = calculate_score(steps, level_time);
	display_level_complete_screen(score);
	
	uint32_t last_ssd_time = get_current_time();
	level_time = 0;
	// Do nothing until a valid input is made.
	while (1)
	{
		uint32_t current_time = get_current_time();
		if (current_time >= last_ssd_time + 10)
		{
			update_ssd_steps(steps);
			last_ssd_time = current_time;
		}
		
		// Get serial input. If no serial input is ready, serial_input
		// would be -1 (not a valid character).
		int serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}

		if (toupper(serial_input) == 'R') {
			// Restart game.
			level = 1;
			steps = 0;
			break;
		} else if (toupper(serial_input) == 'E') {
			// Exit game.
			steps = 0;
			level = 1;
			start_screen();
			break;
		} else if (toupper(serial_input) == 'N') {
			level = 2;
			// Clear serial terminal.
			break;
		}


		
	}
}
