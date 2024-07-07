/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett, Cody Burnett
 * Modified by <YOUR NAME HERE>
 */

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"
#include <string.h> 
#include <stdlib.h>

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

uint32_t is_muted = 0;
char mode[20] = "Basic Moves       ";

// WARNING
// Function prototype for move_is_valid
uint8_t move_is_valid();

void pause_message(void)
{
	move_terminal_cursor(10, 20);
	printf_P(PSTR("Game Paused. Press 'P' to resume."));
}

void clear_pause_message(void)
{
	move_terminal_cursor(10, 20);
	printf_P(PSTR("                                 "));
}

void adc_init() {
	// Set the reference voltage to AVcc
	ADMUX = (1<<REFS0);

	// Enable ADC and set the prescaler to 128
	// ADC frequency will be 8000000 / 128 = 62500Hz
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
}

// Read ADC value from a channel
uint16_t adc_read(uint8_t channel) {
	// Select ADC channel with safety mask
	ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);

	// Start single conversion
	ADCSRA |= (1<<ADSC);

	// Wait for conversion to complete
	while (ADCSRA & (1<<ADSC));

	return ADC;
}

// Thresholds for detecting movement
#define JOYSTICK_DEAD_ZONE 50
#define ADC_MAX 1023
#define ADC_MID 512

// Calculate delay based on joystick position
uint16_t calculate_delay(uint16_t joystick_value) {
	uint16_t offset = abs(joystick_value - ADC_MID);
	if (offset < JOYSTICK_DEAD_ZONE) return 0;  // No movement if within dead zone

	uint16_t delay = 250 - (200 * offset / (ADC_MAX / 2 - JOYSTICK_DEAD_ZONE));
	return delay > 25 ? delay : 25;  // Ensure minimum delay for controllability
}

// Delay for the joystick
void custom_delay_ms(uint16_t milliseconds) {
	while (milliseconds) {
		_delay_ms(1);  
		milliseconds--;
	}
}
// Joystick movement
void move_cursor_with_joystick(void) {
	uint16_t x_pos = adc_read(0);
	uint16_t y_pos = adc_read(1);

	int8_t dx = 0, dy = 0;
	if (x_pos > ADC_MID + JOYSTICK_DEAD_ZONE) dx = 1;
	else if (x_pos < ADC_MID - JOYSTICK_DEAD_ZONE) dx = -1;

	if (y_pos > ADC_MID + JOYSTICK_DEAD_ZONE) dy = 1;
	else if (y_pos < ADC_MID - JOYSTICK_DEAD_ZONE) dy = -1;

	uint16_t delay = calculate_delay(dx ? x_pos : y_pos);
	if (delay) {
		move_cursor(dx, dy);
		custom_delay_ms(delay);
	}
}

// Equals to 1 if the computer firing animation is running
uint8_t animation_running = 0;


/////////////////////////////// main //////////////////////////////////
int main(void)
{
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();
	
	// Make pin OC1B be an output (port D, pin 4)
	DDRD = (1<<4);
	
	// Loop forever and continuously play the game.
	while(1)
	{
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void)
{
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200, 0);
	
	init_timer0();
	init_timer1();
	init_timer2();
	adc_init(); // Initialize the ADC for joystick
	// Turn on global interrupts
	sei();
}

void start_screen(void)
{
	// Clear terminal screen and output a message
	clear_terminal();
	hide_cursor();
	set_display_attribute(FG_WHITE);
	move_terminal_cursor(10,4);
	printf_P(PSTR(" _______    ______  ________  ________  __        ________   ______   __    __  ______  _______  "));
	move_terminal_cursor(10,5);
	printf_P(PSTR("|       \\  /      \\|        \\|        \\|  \\      |        \\ /      \\ |  \\  |  \\|      \\|       \\ "));
	move_terminal_cursor(10,6);
	printf_P(PSTR("| $$$$$$$\\|  $$$$$$\\\\$$$$$$$$ \\$$$$$$$$| $$      | $$$$$$$$|  $$$$$$\\| $$  | $$ \\$$$$$$| $$$$$$$\\"));
	move_terminal_cursor(10,7);
	printf_P(PSTR("| $$__/ $$| $$__| $$  | $$      | $$   | $$      | $$__    | $$___\\$$| $$__| $$  | $$  | $$__/ $$"));
	move_terminal_cursor(10,8);
	printf_P(PSTR("| $$    $$| $$    $$  | $$      | $$   | $$      | $$  \\    \\$$    \\ | $$    $$  | $$  | $$    $$"));
	move_terminal_cursor(10,9);
	printf_P(PSTR("| $$$$$$$\\| $$$$$$$$  | $$      | $$   | $$      | $$$$$    _\\$$$$$$\\| $$$$$$$$  | $$  | $$$$$$$ "));
	move_terminal_cursor(10,10);
	printf_P(PSTR("| $$__/ $$| $$  | $$  | $$      | $$   | $$_____ | $$_____ |  \\__| $$| $$  | $$ _| $$_ | $$      "));
	move_terminal_cursor(10,11);
	printf_P(PSTR("| $$    $$| $$  | $$  | $$      | $$   | $$     \\| $$     \\ \\$$    $$| $$  | $$|   $$ \\| $$      "));
	move_terminal_cursor(10,12);
	printf_P(PSTR(" \\$$$$$$$  \\$$   \\$$   \\$$       \\$$    \\$$$$$$$$ \\$$$$$$$$  \\$$$$$$  \\$$   \\$$ \\$$$$$$ \\$$      "));
	move_terminal_cursor(10,14);
	// change this to your name and student number; remove the chevrons <>
	printf_P(PSTR("CSSE2010/7201 Project by <Haozhi Ryan Yang> - <46968096>"));
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	show_start_screen();

	uint32_t last_screen_update, current_time;
	last_screen_update = get_current_time();
	
	int8_t frame_number = -2*ANIMATION_DELAY;
	
	move_terminal_cursor(0, 40);
	printf("MODE: %s", mode);

	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1)
	{
		// First check for if a 's' or a is pressed
		// There are two steps to this
		// 1) collect any serial input (if available)
		// 2) check if the input is equal to the character 's' or 'a'
		char serial_input = -1;
		if (serial_input_available())
		{
			serial_input = fgetc(stdin);
		}
				
		if (serial_input == 's' || serial_input == 'S') {
			 
			break;
		}
		
		if (serial_input == 'y' || serial_input == 'Y') {
			if (strcmp(mode, "Basic Moves       ") == 0) {
				move_terminal_cursor(0, 40);
				strcpy(mode, "Search and Destroy");
				printf("MODE: %s", mode);
			} else {
				strcpy(mode, "Basic Moves       ");
				move_terminal_cursor(0, 40);
				printf("MODE: %s", mode);
			}
			
		}
		
		
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED)
		{
			break;
		}

		// every 200 ms, update the animation
		current_time = get_current_time();
		if (current_time - last_screen_update > 200)
		{
			update_start_screen(frame_number);
			frame_number++;
			if (frame_number > ANIMATION_LENGTH)
			{
				frame_number -= ANIMATION_LENGTH+ANIMATION_DELAY;
			}
			last_screen_update = current_time;
		}
	}
}

void new_game(void)
{
	// Clear the serial terminal
	clear_terminal();
	
	// Initialize the game and display
	initialise_game();
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
	
}

void play_game(void)
{
	uint32_t last_flash_time, current_time;
	int8_t btn; 
	
	last_flash_time = get_current_time();
	
	// time at when the computer's ships were revealed
	uint32_t reveal_start_time = 0;
	
	uint8_t reveal_end_time = 0;
	uint8_t game_paused = 0;
	uint32_t pause_start_time = 0;
	uint32_t pause_duration = 0;
	uint32_t cheat_used = 0;
	  
	// We play the game until it's over
	while (!is_game_over())
	{
		
			
			// Handle joystick movement
			move_cursor_with_joystick();
		
			// We need to check if any button has been pushed, this will be
			// NO_BUTTON_PUSHED if no button has been pushed
			// Checkout the function comment in `buttons.h` and the implementation
			// in `buttons.c`.
			btn = button_pushed();
		
			if (btn == BUTTON0_PUSHED)
			{
				// move the cursor
				// see move_cursor(...) in game.c
				// remember to reset the cursor flashing cycle
		
				move_cursor(1, 0);
			}
			// repeat for the other buttons
			// combine with serial inputs
		
			if (btn == BUTTON1_PUSHED)
			{
				// move the cursor
				// see move_cursor(...) in game.c
				// remember to reset the cursor flashing cycle
			
				move_cursor(0, -1);
			}
		
			if( btn == BUTTON2_PUSHED)
			{
				// move the cursor
				// see move_cursor(...) in game.c
				// remember to reset the cursor flashing cycle
			
				move_cursor(0, 1);
			}
		
			if (btn == BUTTON3_PUSHED)
			{
				// move the cursor
				// see move_cursor(...) in game.c
				// remember to reset the cursor flashing cycle
			
				move_cursor(-1, 0);
			}
		
			current_time = get_current_time();
			if (current_time - pause_duration >= last_flash_time + 200)
			{
				// 200ms (0.2 second) has passed since the last time we advance the
				// notes here, so update the advance the notes
				flash_cursor();
				
				pause_duration = 0;
				
				// Update the most recent time the notes were advance
				last_flash_time = current_time;
			}
		
			char serial_input = -1;
			if (serial_input_available())
			{
				serial_input = fgetc(stdin);
			}
		
			if (serial_input == 's' || serial_input == 'S')
			{
				// Move cursor with terminal input
				move_cursor(0, -1);
			} else if (serial_input == 'w' || serial_input == 'W') {
				// Move cursor with terminal input
				move_cursor(0, 1);
			} else if (serial_input == 'a' || serial_input == 'A') {
				// Move cursor with terminal input
				move_cursor(-1, 0);
			} else if (serial_input == 'd' || serial_input == 'D') {
				// Move cursor with terminal input
				move_cursor(1, 0);
			} else if (serial_input == 'f' || serial_input == 'F') {
				// Fire at location
				if (move_is_valid()) {
					fire_at_location(0, 0);
					computer_turn();	
				}
			} else if (serial_input == 'c' || serial_input == 'C') {
				// Reveals the computer's ships
				reveal_start_time = get_current_time();
				reveal_computer_ships();	
				reveal_end_time = 1;
			} else if (serial_input == 'p' || serial_input == 'P') {
				// Pauses the game
				game_paused = 1;
				pause_start_time = get_current_time();
				pause_duration = 0;
				clear_invalid_move_message();
				pause_message();
			} else if (serial_input == 'b' || serial_input == 'B') {
				if (cheat_used == 0) {
					// Fires at the location and its surroundings
					fire_around_location();
					computer_turn();
					cheat_used ++;
				} else {
					invalid_move_message();
				}
			} else if (serial_input == 'n' || serial_input == 'N') {
				if (cheat_used == 0) {
					// Fires at the location and its row
					fire_in_row();
					computer_turn();
					cheat_used ++;
					} else {
					invalid_move_message();
				}
			} else if (serial_input == 'm' || serial_input == 'M') {
				if (cheat_used == 0) {
					// Fires at the location and its column
					fire_in_column();
					computer_turn();
					cheat_used ++;
					} else {
					invalid_move_message();
				}
			} else if (serial_input == 'q' || serial_input == 'Q') {
				// mute/unmute the sound
				if (is_muted) {
					is_muted = 0;
				} else {
					is_muted = 1;
				}
			}
		
			// Hides the computer's ships after 1 second
			if (reveal_end_time && current_time - reveal_start_time >= 1000) {
				hide_computer_ships();
				reveal_end_time = 0;
			}
		
			// Handles the pause
			while (game_paused) {
				char serial_input = -1;
				if (serial_input_available())
				{
					serial_input = fgetc(stdin);
				}
			
				if (serial_input == 'p' || serial_input == 'P')
				{
					pause_duration += get_current_time() - pause_start_time;
					game_paused = 0;
					clear_pause_message();
				}
			
			}
		
		
	}
	// We get here if the game is over.
}

void handle_game_over()
{
	move_terminal_cursor(10,14);
	printf_P(PSTR("GAME OVER"));
	move_terminal_cursor(10,15);
	printf_P(PSTR("Press a button or 's'/'S' to start a new game"));
		
	// Do nothing until a button or 's'/'S' are pushed should also start a
	// new game
	while (1) {
			char serial_input = -1;
			if (serial_input_available())
			{
				serial_input = fgetc(stdin);
			}
			// If the serial input is 's', then exit the start screen
			if (serial_input == 's' || serial_input == 'S')
			{
				break;
			}
			// Next check for any button presses
			int8_t btn = button_pushed();
			if (btn != NO_BUTTON_PUSHED)
			{
				break;
			}
	}
	
	start_screen();

}
