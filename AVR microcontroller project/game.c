/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett, Cody Burnett
 */ 
#define F_CPU 8000000UL // WARNING
#include <avr/io.h> 
#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "ledmatrix.h"
#include "terminalio.h"
#include <util/delay.h> // delete this is for the delay for the buzzer
#include <string.h> // WARNING

uint16_t freq_to_clock_period(uint16_t freq) {
	// Converts frequency to clock period in cycles for a 1MHz clock
	return (1000000UL / freq);  
}

uint16_t duty_cycle_to_pulse_width(float dutycycle, uint16_t clockperiod) {
	// Converts duty cycle to pulse width in clock cycles
	return (dutycycle * clockperiod) / 100;  
}

uint8_t human_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS];
uint8_t computer_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS];
uint8_t cursor_x, cursor_y;
uint8_t cursor_on;

// The string for the computer's mode
// Search and Destroy
extern char mode[20];

// Determines if the game is muted/unmuted
extern uint32_t is_muted;

extern uint8_t animation_running;

// Initialize the game by resetting the grid and beat
void initialise_game(void)
{
	// clear the splash screen art
	ledmatrix_clear();
	
	// see "Human Turn" feature for how ships are encoded
	// fill in the grid with the ships
	uint8_t initial_human_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS] =
		{{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {SEA,                  CARRIER|HORIZONTAL|SHIP_END,    CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL, CARRIER|HORIZONTAL, CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL|SHIP_END,    SEA                 },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {SEA,                  SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            SEA                 },
		 {DESTROYER|SHIP_END,   SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            FRIGATE|SHIP_END    },
		 {DESTROYER,            SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            FRIGATE             },
		 {DESTROYER|SHIP_END,   SEA,                            CRUISER|HORIZONTAL|SHIP_END,    CRUISER|HORIZONTAL, CRUISER|HORIZONTAL, CRUISER|HORIZONTAL|SHIP_END,    SEA,                            FRIGATE|SHIP_END    },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 }};
	uint8_t initial_computer_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS] =
		{{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {DESTROYER|SHIP_END,   SEA,                            CRUISER|HORIZONTAL|SHIP_END,    CRUISER|HORIZONTAL, CRUISER|HORIZONTAL, CRUISER|HORIZONTAL|SHIP_END,    SEA,                            FRIGATE|SHIP_END    },
		 {DESTROYER,            SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            FRIGATE             },
		 {DESTROYER|SHIP_END,   SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            FRIGATE|SHIP_END    },
		 {SEA,                  SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            SEA                 },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
		 {SEA,                  CARRIER|HORIZONTAL|SHIP_END,    CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL, CARRIER|HORIZONTAL, CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL|SHIP_END,    SEA                 },
		 {SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 }};
	
	// Initializes the default
	for (uint8_t i=0; i<GRID_NUM_COLUMNS; i++)
	{
		for (uint8_t j=0; j<GRID_NUM_COLUMNS; j++)
		{
			human_grid[j][i] = initial_human_grid[j][i];
			computer_grid[j][i] = initial_computer_grid[j][i];
			if (human_grid[j][i] & SHIP_MASK)
			{
				ledmatrix_draw_pixel_in_human_grid(i, j, COLOUR_ORANGE);
			}
		}
	}

	cursor_x = 3;
	cursor_y = 3;
	cursor_on = 1;
}

// A bitmask that marks a cell as hit
#define HIT_MASK 128
#define MISS_MASK 64
#define SUNK_MASK 32

const char* ship_names[] = {
	"Sea", "Carrier", "Cruiser", "Destroyer", "Frigate", "Corvette", "Submarine"
};

// Handles the flashing of the cursor
void flash_cursor(void)
{
	cursor_on = 1-cursor_on;
	if (cursor_on){
		if (computer_grid[cursor_y][cursor_x] & (HIT_MASK | MISS_MASK)) {
			// Flash dark yellow if cursor is at a location that cannot be fired at
			ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_DARK_YELLOW);
		} else {
			ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_YELLOW);
		}
		
	} else if (computer_grid[cursor_y][cursor_x] & (MISS_MASK)) {
		// If the cursor is on a location that was fired at and missed
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_GREEN);
	} else if (computer_grid[cursor_y][cursor_x] & (SUNK_MASK)) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_DARK_RED);	
	} else if (computer_grid[cursor_y][cursor_x] & (HIT_MASK)) {
		// If the cursor is on a location that was fired at and hit
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_RED);	
	} else {
		// If the cursor is on the sea
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_BLACK);
	}
}

// moves the position of the cursor by (dx, dy) such that if the cursor
// started at (cursor_x, cursor_y) then after this function is called,
// it should end at ( (cursor_x + dx) % WIDTH, (cursor_y + dy) % HEIGHT)
// the cursor should be displayed after it is moved as well
void move_cursor(int8_t dx, int8_t dy) {
	
	//YOUR CODE HERE
	/*suggestions for implementation:
	 * 1: remove the display of the cursor at the current location
	 *		(and replace it with whatever piece is at that location)
	 * 2: update the positional knowledge of the cursor, this will include
	 *		variables cursor_x, cursor_y and cursor_visible. Make sure you
	 *		consider what should happen if the cursor moves off the board.
	 * 3: display the cursor at the new location
	 * 4: reset the cursor flashing cycle. See project.c for how the cursor
	 *		is flashed.
	 */
	
	#define WIDTH 8       
	#define HEIGHT 8
	
	if (computer_grid[cursor_y][cursor_x] & SUNK_MASK) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_DARK_RED);
	} else if (computer_grid[cursor_y][cursor_x] & HIT_MASK) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_RED);
	} else if (computer_grid[cursor_y][cursor_x] & MISS_MASK) {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_GREEN);
	} else {
		ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_BLACK);
	}
	
	// Update the position of the cursor	
	cursor_y = (cursor_y + dy + HEIGHT) % HEIGHT;
	cursor_x = (cursor_x + dx + WIDTH) % WIDTH;  
	

	// Display the cursor at the new location
	ledmatrix_draw_pixel_in_computer_grid(cursor_x, cursor_y, COLOUR_YELLOW);
	
	// Reset the cursor flashing cycle
	cursor_on = 1;
	flash_cursor();
	
}

// The number of consecutive invalid moves the user has made
int invalid_move_count = 0;

// Message for invalid move
void invalid_move_message() {
	const char* messages[] = {
		"Invalid move, please try again.",
		"Select a different target!       ",
		"Seriously? Select a different target!"
	};
	
	// Counts the amount of messages
	const int message_count = sizeof(messages) / sizeof(messages[0]);
	
	// Resets the location of the cursor
	move_terminal_cursor(0, 20); 
	
	// Determines how many times the user has made an invalid move
	if (invalid_move_count < message_count) {
		printf("%s\n", messages[invalid_move_count]); 
		} else {
		printf("%s\n", messages[message_count - 1]); 
	}
	invalid_move_count++;
}

// Tracks how many invalid moves the user has attempted in a row
void reset_invalid_move() {
	invalid_move_count = 0; 
}

// Clears the message
void clear_invalid_move_message() {
	move_terminal_cursor(0, 20);  
	printf("                                               \n");
}

typedef struct {
	const char* name;
	int size;
	int hits;
	int sunk;
} Ship;

Ship human_ships[] = {
	{"Carrier", 6, 0, 0},
	{"Cruiser", 4, 0, 0},
	{"Destroyer", 3, 0, 0},
	{"Frigate", 3, 0, 0},
	{"Corvette", 2, 0, 0},
	{"Submarine", 2, 0, 0}
};

Ship computer_ships[] = {
	{"Carrier", 6, 0, 0},
	{"Cruiser", 4, 0, 0},
	{"Destroyer", 3, 0, 0},
	{"Frigate", 3, 0, 0},
	{"Corvette", 2, 0, 0},
	{"Submarine", 2, 0, 0}
};

const char* initial_names[] = {
	"Carrier", "Cruiser", "Destroyer", "Frigate", "Corvette", "Submarine"
};
int initial_sizes[] = {6, 4, 3, 3, 2, 2};
	
void reset_ships(Ship* ships) {
	for (int i = 0; i < 6; i++) {
		ships[i].name = initial_names[i];
		ships[i].size = initial_sizes[i];
		ships[i].hits = 0;
		ships[i].sunk = 0;
	}
}

static int human_message_line = 0;
static int computer_message_line = 0;
const int message_area_start = 1;
const int message_area_width = 40;

// Sends a message to the terminal when the ship is sunk
void sunk_ship_message(const char* player, const char* ship) {


	if (strcmp(player, "human") == 0) {
		// Message for when the player sinks a computer's ship
		move_terminal_cursor(0, message_area_start + human_message_line);
		printf("I Sunk Your %s", ship);
		human_message_line++;
		} else {
		// Message for when the computer sinks the human's ship
		move_terminal_cursor(message_area_width, message_area_start + computer_message_line);
		printf("You Sunk My %s", ship);
		computer_message_line++;
	}
}

// Handles the sinking of ships
void check_sunk_ships(Ship* ships, uint8_t grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS], const char* player) {
	for (int i = 0; i < sizeof(human_ships) / sizeof(human_ships[0]); i++) {
		if (ships[i].sunk == 0 && ships[i].hits == ships[i].size) {
			// Make the ship status as sunk
			ships[i].sunk = 1;
			
			// Plays a  sound for when the ship is sunk
			if (!is_muted && (strcmp(player, "computer") == 0)) {
				play_sound("computer sink");
				
			} else if (!is_muted) {
				play_sound("human sink");
			}
			
			// Prints a message when a ship is sunk
			sunk_ship_message(player, ships[i].name);
			for (int y = 0; y < GRID_NUM_ROWS; y++) {
				for (int x = 0; x < GRID_NUM_COLUMNS; x++) {
					if ((grid[y][x] & SHIP_MASK) == i + 1) {
						// Labels the ship as hit and sunk
						grid[y][x] |= (HIT_MASK | SUNK_MASK);
						if (strcmp(player, "computer") == 0) {
							// Makes the computer's ship dark to symbolist it being hit
							ledmatrix_draw_pixel_in_computer_grid(x, y, COLOUR_DARK_RED);
						} else {
							// Makes the player's ship dark to symbolize it being hit
							ledmatrix_draw_pixel_in_human_grid(x, y, COLOUR_DARK_RED);
						}
					}
				}
			}
		} 
	} 
}

// The default coordinates for where the computer is shooting in Basic mode
// Starts at the top left corner
// Can be overwritten in search and destroy mode
int computer_target_x = 0;
int computer_target_y = 7;

// Computer starts in search mode
int is_search_mode = 1;

// Checks if the randomly generated coordinate is valid
// Implemented in search mode in computer_turn()
static uint8_t is_valid_coordinate(uint8_t x, uint8_t y) {
	return x < GRID_NUM_COLUMNS && y < GRID_NUM_ROWS;
}

// Contains the cells that still needs to be fired at by the computer in destroy mode
static uint8_t cells_to_hit[GRID_NUM_ROWS * GRID_NUM_COLUMNS][2];

static uint8_t cells_to_hit_count = 0;

static const int8_t directions[4][2] = {
	{1, 0},  // Right
	{0, 1},  // Down
	{-1, 0}, // Left
	{0, -1}  // Up
};

static uint8_t last_hit_x = 0;
static uint8_t last_hit_y = 0;

// Adds the adjacent cells that still needs to be hit into the cells_to_hit array
void add_adjacent_cells_to_hit(uint8_t x, uint8_t y) {
	for (int i = 0; i < 4; i++) {
		uint8_t adj_x = x + directions[i][0];
		uint8_t adj_y = y + directions[i][1];
		
		if (is_valid_coordinate(adj_x, adj_y) && !(human_grid[adj_y][adj_x] & (HIT_MASK | MISS_MASK))) {
			cells_to_hit[cells_to_hit_count][0] = adj_x;
			cells_to_hit[cells_to_hit_count][1] = adj_y;
			cells_to_hit_count++;
		}
	}
}

// Firing animation
void computer_fire_animation(uint8_t target_x, uint8_t target_y) {
	// Example animation: flash the target location a few times
	animation_running = 1;
	for (int i = 0; i < 3; i++) {
		// Flash target location
		ledmatrix_draw_pixel_in_human_grid(target_x, target_y, COLOUR_YELLOW);
		_delay_ms(50);
		// Turn off the flash
		ledmatrix_draw_pixel_in_human_grid(target_x, target_y, COLOUR_RED);
		_delay_ms(50);
	}
	animation_running = 0;
}

void computer_turn() {
	// Computer turn in basic mode
	if (strcmp(mode, "Basic Moves       ") == 0) {
		uint8_t cell_value = human_grid[computer_target_y][computer_target_x];
		
		 // Play animation
		 computer_fire_animation(computer_target_x, computer_target_y);
		 
		// Colors the grid
		if (human_grid[computer_target_y][computer_target_x] & SHIP_MASK) {
			// Plays sound
			if (!is_muted) {
				play_sound("human hit");
			}
			
			// Add hits to the ship
			uint8_t ship_type = cell_value & SHIP_MASK;
			human_ships[ship_type - 1].hits++;
		
			// Marks the target as hit
			human_grid[computer_target_y][computer_target_x] |= HIT_MASK;
		
			// Colors it red since there is a ship
			ledmatrix_draw_pixel_in_human_grid(computer_target_x, computer_target_y, COLOUR_RED);
		
			// Checks if a player's ship is sunk
			check_sunk_ships(human_ships, human_grid, "human");
			
			} else {
			
				// Marks the target as missed
				human_grid[computer_target_y][computer_target_x] |= MISS_MASK;
			
				// Colors it green if there is no ship
				ledmatrix_draw_pixel_in_human_grid(computer_target_x, computer_target_y, COLOUR_GREEN);
		}
	
		// Move to the next target position
		computer_target_x++;
		if (computer_target_x >= GRID_NUM_COLUMNS) {
			computer_target_x = 0;
			computer_target_y--;
		}
		
	} else  if (strcmp(mode, "Search and Destroy") == 0) {
		// Logic for Search and Destroy mode
		if (is_search_mode) {
			
			// Search mode
			do {
				 computer_target_x = rand() % GRID_NUM_COLUMNS;
				 computer_target_y = rand() % GRID_NUM_ROWS;
				 
			} while (!is_valid_coordinate(computer_target_x, computer_target_y) || (human_grid[computer_target_y][computer_target_x] & (HIT_MASK | MISS_MASK)));
			
			 if (!is_valid_coordinate(computer_target_x, computer_target_y)) {
				 return;
			 }

			uint8_t cell_value = human_grid[computer_target_y][computer_target_x];
			
			// Play animation
			computer_fire_animation(computer_target_x, computer_target_y);
			
			if (human_grid[computer_target_y][computer_target_x] & SHIP_MASK) {
				// Plays sound
				if (!is_muted) {
					play_sound("human hit");
				}
				
				// Enters destroy mode
				is_search_mode = 0;
				
				// Add hits to the ship
				uint8_t ship_type = cell_value & SHIP_MASK;
				human_ships[ship_type - 1].hits++;
				
				// Marks the target as hit
				human_grid[computer_target_y][computer_target_x] |= HIT_MASK;
				
				// Colors it red if there is a ship
				ledmatrix_draw_pixel_in_human_grid(computer_target_x, computer_target_y, COLOUR_RED);
				
				// Checks if a player's ship is sunk
				check_sunk_ships(human_ships, human_grid, "human");
				
				// Add adjacent cells to the cells_to_hit array
				add_adjacent_cells_to_hit(computer_target_x, computer_target_y);

				} else {
				// Remains in search mode
				is_search_mode = 1;
					
				// Marks the target as a miss
				human_grid[computer_target_y][computer_target_x] |= MISS_MASK;
				
				// Colors it green
				ledmatrix_draw_pixel_in_human_grid(computer_target_x, computer_target_y, COLOUR_GREEN);
			}
		
		} else if (!is_search_mode) {
			// Destroy mode
			
			 // Initialize x and y to default values
			 uint8_t x = 0, y = 0;
			 
			if (cells_to_hit_count > 0) {
				// Select randomly from cells_to_hit array
				uint8_t index = rand() % cells_to_hit_count;
				x = cells_to_hit[index][0];
				y = cells_to_hit[index][1];

				// Remove the selected cell from the array
				cells_to_hit_count--;
				for (int i = index; i < cells_to_hit_count; i++) {
					cells_to_hit[i][0] = cells_to_hit[i + 1][0];
					cells_to_hit[i][1] = cells_to_hit[i + 1][1];
				}
			}
			
			 uint8_t cell_value = human_grid[y][x];
			 
			// Play animation
			computer_fire_animation(x, y);

			 if (human_grid[y][x] & SHIP_MASK) {
				 // Plays sound
				 if (!is_muted) {
					 play_sound("human hit");
				 }
				 
				 // Add hits to the ship
				 uint8_t ship_type = cell_value & SHIP_MASK;
				 human_ships[ship_type - 1].hits++;
				 
				 // Marks the target as hit
				 human_grid[y][x] |= HIT_MASK;
				 
				 // Colors it red if there is a ship
				 ledmatrix_draw_pixel_in_human_grid(x, y, COLOUR_RED);
				 
				 // Checks if a player's ship is sunk
				 check_sunk_ships(human_ships, human_grid, "human");
				
				 // Add adjacent cells to the cells_to_hit array
				 add_adjacent_cells_to_hit(x, y);

				 // Enter destroy mode
				 is_search_mode = 0;
				 last_hit_x = x;
				 last_hit_y = y;
				 
				 } else {
				 // Miss logic
				 human_grid[y][x] |= MISS_MASK;
				 ledmatrix_draw_pixel_in_human_grid(x, y, COLOUR_GREEN);

				 if (cells_to_hit_count == 0) {
					 is_search_mode = 1;  
				 }
			 }			
		}	
	}
}

// Returns 1 if the move is valid
uint8_t move_is_valid() {
	uint8_t target_x = cursor_x;
	uint8_t target_y = cursor_y;
	// Check if cursor on the computer's grid contains a ship	
	if (computer_grid[target_y][target_x] & (HIT_MASK | MISS_MASK)) {
		// The firing location is invalid
		invalid_move_message();
		return 0;
	} else {
		return 1;
	}
}

void fire_at_location(int8_t dx, int8_t dy) {
	uint8_t target_x = cursor_x + dx;
	uint8_t target_y = cursor_y + dy;
	uint8_t cell_value = computer_grid[target_y][target_x];
	
	if (computer_grid[target_y][target_x] & SHIP_MASK) {
		// Mark location as hit if it's not already fired upon
		computer_grid[target_y][target_x] |= HIT_MASK;
			
		// Add hits to the ship
		uint8_t ship_type = cell_value & SHIP_MASK;
		computer_ships[ship_type - 1].hits++;
		

		// Plays a  sound for when the ship is hit but not sunk
		if (!is_muted) {
			play_sound("computer hit");
		}	
			
		// Check if a computer's ship is sunk
		check_sunk_ships(computer_ships, computer_grid, "computer");
			
		clear_invalid_move_message();
		reset_invalid_move();

	} else {
		// Mark location as miss
		computer_grid[target_y][target_x] |= MISS_MASK;
		
		clear_invalid_move_message();
		reset_invalid_move();
	}
}

// Logic for playing the sound effect
void play_sound(const char *event) {

	if (strcmp(event, "computer hit") == 0) {
		// Sound effect for when the human hits the computer's ship
		// Single tone 
		
		// First beep
		uint16_t freq = 200; // Hz
		float dutycycle = 50; // 50%
		uint16_t clockperiod = freq_to_clock_period(freq);
		uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
		
		sound_on(freq, dutycycle, clockperiod, pulsewidth);
		_delay_ms(50);		
		sound_off();
		
				
	} else if (strcmp(event, "human hit") == 0) {
		// Sound effect for when the computer hits the humans ship
		// Single tone
		// More "corrupted"-sounding than human hit
		
		uint16_t baseFreq = 100; // Hz
		float baseDutyCycle = 50; // 50%
		uint16_t duration = 200; // ms

		uint16_t clockperiod = freq_to_clock_period(baseFreq);
		uint16_t pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		for (uint16_t i = 0; i < duration; ++i) {
			// Introduce more variation for a corrupted effect
			uint16_t freq = baseFreq + (rand() % (baseFreq / 5)) - (baseFreq / 10);
			float dutycycle = baseDutyCycle + ((rand() % 20) - 10) / 100.0;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		sound_off();
		
	} else if (strcmp(event, "computer sink") == 0) {
		// Sound effect for when the human sinks the computer's ship
		// Series of tones
		// Fancier tone than "hit"
		
		uint16_t freq = 200; // Hz
		float dutycycle = 50; // 50%
		uint16_t clockperiod = freq_to_clock_period(freq);
		uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
		
		sound_on(freq, dutycycle, clockperiod, pulsewidth);
		_delay_ms(100);
		
		freq = 300; // Hz
		dutycycle = 50; // 50%
		clockperiod = freq_to_clock_period(freq);
		pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
		
		sound_on(freq, dutycycle, clockperiod, pulsewidth);
		_delay_ms(100);
		
		sound_off();
		
	} else if (strcmp(event, "human sink") == 0) {
		// Sound effect for when the computer sinks the humans ship
		// Series of tones
		// Fancier tone than "hit"
		
		// First beep
		uint16_t baseFreq = 400; // Hz
		float baseDutyCycle = 50; // 50%
		uint16_t duration = 200; // ms

		uint16_t clockperiod = freq_to_clock_period(baseFreq);
		uint16_t pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		// Second higher pitched beep
		baseFreq = 600; // Hz
		baseDutyCycle = 50; // 50%
		duration = 200; // ms

		clockperiod = freq_to_clock_period(baseFreq);
		pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		sound_off();
		
	} else if (strcmp(event, "human wins") == 0) {
		// Sound effect for when the human wins
		// Series of tones
		// Fancier than "sink"
			uint16_t freq = 800; // Hz
			float dutycycle = 50; // 50%
			uint16_t clockperiod = freq_to_clock_period(freq);
			uint16_t pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);
			
			sound_on(freq, dutycycle, clockperiod, pulsewidth);
			_delay_ms(250);
			sound_off();
			
			_delay_ms(50);
			sound_on(freq, dutycycle, clockperiod, pulsewidth);
			_delay_ms(100);
			sound_off();
			
			_delay_ms(50);
			sound_on(freq, dutycycle, clockperiod, pulsewidth);
			_delay_ms(100);
			sound_off();
			
			_delay_ms(50);
			sound_on(freq, dutycycle, clockperiod, pulsewidth);
			_delay_ms(800);
			
			sound_off();
		
	} else if (strcmp(event, "computer wins") == 0) {
		// Sound effect for when the computer wins
		// Series of tones
		// Fancier than "sink"
		// First beep
		uint16_t baseFreq = 500; // Hz
		float baseDutyCycle = 50; // 50%
		uint16_t duration = 300; // ms

		uint16_t clockperiod = freq_to_clock_period(baseFreq);
		uint16_t pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		
		// Second lower pitched beep
		baseFreq = 400; // Hz
		baseDutyCycle = 50; // 50%
		duration = 300; // ms

		clockperiod = freq_to_clock_period(baseFreq);
		pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		// Third even lower pitched beep
		baseFreq = 300; // Hz
		baseDutyCycle = 50; // 50%
		duration = 300; // ms

		clockperiod = freq_to_clock_period(baseFreq);
		pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		// Fourth even lower pitched beep
		baseFreq = 200; // Hz
		baseDutyCycle = 50; // 50%
		duration = 300; // ms

		clockperiod = freq_to_clock_period(baseFreq);
		pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		// Fifth lowest pitched beep
		baseFreq = 100; // Hz
		baseDutyCycle = 50; // 50%
		duration = 300; // ms

		clockperiod = freq_to_clock_period(baseFreq);
		pulsewidth = duty_cycle_to_pulse_width(baseDutyCycle, clockperiod);
		for (uint16_t i = 0; i < duration; ++i) {
			uint16_t freq = baseFreq + (rand() % (baseFreq / 10)) - (baseFreq / 20);
			float dutycycle = baseDutyCycle + ((rand() % 10) - 5) / 100;

			clockperiod = freq_to_clock_period(freq);
			pulsewidth = duty_cycle_to_pulse_width(dutycycle, clockperiod);

			sound_on(freq, dutycycle, clockperiod, pulsewidth);
		}
		
		sound_off();
	}

}

// The sound itself in its on state
void sound_on(uint16_t freq, float dutycycle, uint16_t clockperiod, uint16_t pulsewidth) {
		
	OCR1A = clockperiod - 1;
	OCR1B = (pulsewidth > 0) ? (pulsewidth - 1) : 0;

	// Set up Timer/Counter 1 for Fast PWM, counting from 0 to the value in OCR1A
	// before resetting to 0. Count at 1MHz (CLK/8).
	TCCR1A = (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
	TCCR1B = (1 << WGM13) | (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10);
		
	
}

// Turns off the sound
void sound_off() {
	// Turn off the PWM
	TCCR1A &= ~(1 << COM1B1);
}

// Fires at a location and its surrounding cells
void fire_around_location() {
	for (int8_t dy = -1; dy <= 1; dy++) {
		for (int8_t dx = -1; dx <= 1; dx++) {

			int target_x = cursor_x + dx;
			int target_y = cursor_y + dy;

			// Ensures the target is within the grid
			if (target_x < 0 || target_x >= GRID_NUM_COLUMNS || target_y < 0 || target_y >= GRID_NUM_ROWS) {
				continue;
			}

			//uint8_t cell_value = computer_grid[target_y][target_x];
			
			// Fires at the location
			fire_at_location(dx, dy);

			if (computer_grid[target_y][target_x] & SUNK_MASK) {
				ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_DARK_RED);
				} else if (computer_grid[target_y][target_x] & HIT_MASK) {
				ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_RED);
				} else if (computer_grid[target_y][target_x] & MISS_MASK) {
				ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_GREEN);
				} 
		}
	}
}

// Fires at a location and its entire row
void fire_in_row() {
	int target_y = cursor_y;
	
	// Fire at every location in the current row
	for (int8_t x = 0; x < GRID_NUM_ROWS; x++)
	{
		int target_x = x;
		fire_at_location(-cursor_x + x, 0);
		if (computer_grid[target_y][target_x] & SUNK_MASK) {
			ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_DARK_RED);
			} else if (computer_grid[target_y][target_x] & HIT_MASK) {
			ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_RED);
			} else if (computer_grid[target_y][target_x] & MISS_MASK) {
			ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_GREEN);
		}
	}
}

void fire_in_column() {
	int target_x = cursor_x;
		
	// Fire at every location in the current row
	for (int8_t y = 0; y < GRID_NUM_COLUMNS; y++)
	{
		int target_y = y;
		fire_at_location(0, -cursor_y + y);
		if (computer_grid[target_y][target_x] & SUNK_MASK) {
			ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_DARK_RED);
			} else if (computer_grid[target_y][target_x] & HIT_MASK) {
			ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_RED);
			} else if (computer_grid[target_y][target_x] & MISS_MASK) {
			ledmatrix_draw_pixel_in_computer_grid(target_x, target_y, COLOUR_GREEN);
		}
	}
}

// Lights are all the unlit LEDs when game is over
void game_over_board(Ship* ships, uint8_t grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS], const char* player) {
	for (int i = 0; i < sizeof(human_ships) / sizeof(human_ships[0]); i++) {
		for (int y = 0; y < GRID_NUM_ROWS; y++) {
			for (int x = 0; x < GRID_NUM_COLUMNS; x++) {
				uint8_t cell_value = grid[y][x];
				
				// Colors every location that has not been fired at
				if (!(cell_value & (HIT_MASK | MISS_MASK | SUNK_MASK))) {
					if (cell_value & (SHIP_MASK)) {
						if (strcmp(player, "computer") == 0) {
							ledmatrix_draw_pixel_in_computer_grid(x, y, COLOUR_DARK_ORANGE);
							} else {
							ledmatrix_draw_pixel_in_human_grid(x, y, COLOUR_DARK_ORANGE);
						}
					} else {
						if (strcmp(player, "computer") == 0) {
							ledmatrix_draw_pixel_in_computer_grid(x, y, COLOUR_DARK_GREEN);
							} else {
							ledmatrix_draw_pixel_in_human_grid(x, y, COLOUR_DARK_GREEN);
						}
					}
					
				}
           
			}
			
		}
	}
	
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void)
{	
	int computer_ships_sunk = 0;
	
	// Checks how many ships the computer has alive
	 for (int i = 0; i < 6; i++) {
		 // Checks if all ships are sunk
		 if (computer_ships[i].sunk == 1) {
			 computer_ships_sunk ++;
		 }
	 }
	 
	 if (computer_ships_sunk == 6) {
		 game_over_board(human_ships, human_grid, "human");
		 game_over_board(computer_ships, computer_grid, "computer");
		 move_terminal_cursor(10,13);
		 printf("The player is the winner!");
		 reset_game();
		 if (!is_muted) {
			 play_sound("human wins");
		 }
		 return 1;
	 }
	 
	 int player_ships_sunk = 0;
	 
	 // Checks how many ships the player has alive
	 for (int i = 0; i < 6; i++) {
		 // Checks if all ships are sunk
		 if (human_ships[i].sunk == 1) {
			 player_ships_sunk ++;
		 }
	 }
	 
	  if (player_ships_sunk == 6) {
		  game_over_board(human_ships, human_grid, "human");
		  game_over_board(computer_ships, computer_grid, "computer");
		  move_terminal_cursor(10,13);
		  printf("The computer is the winner!");
		  reset_game();
		  // Plays sound
		  if (!is_muted) {
			  play_sound("computer wins");
		  }
		  return 1;
	  }
	 
	
	// Detect if the game is over i.e. if a player has won.
	return 0;
}

// resets the game to its initial state
void reset_game() {
	reset_ships(human_ships);
	reset_ships(computer_ships);
	
	computer_target_x = 0;
	computer_target_y = 7;
	
	human_message_line = 0;
	computer_message_line = 0;
	
	// WARNING: UNUSED GRID
	/**
	// Reset the grids
	uint8_t initial_human_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS] =
	{{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
	{SEA,                  CARRIER|HORIZONTAL|SHIP_END,    CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL, CARRIER|HORIZONTAL, CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL|SHIP_END,    SEA                 },
	{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
	{SEA,                  SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            SEA                 },
	{DESTROYER|SHIP_END,   SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            FRIGATE|SHIP_END    },
	{DESTROYER,            SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            FRIGATE             },
	{DESTROYER|SHIP_END,   SEA,                            CRUISER|HORIZONTAL|SHIP_END,    CRUISER|HORIZONTAL, CRUISER|HORIZONTAL, CRUISER|HORIZONTAL|SHIP_END,    SEA,                            FRIGATE|SHIP_END    },
	{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 }};
	uint8_t initial_computer_grid[GRID_NUM_ROWS][GRID_NUM_COLUMNS] =
	{{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
	{DESTROYER|SHIP_END,   SEA,                            CRUISER|HORIZONTAL|SHIP_END,    CRUISER|HORIZONTAL, CRUISER|HORIZONTAL, CRUISER|HORIZONTAL|SHIP_END,    SEA,                            FRIGATE|SHIP_END    },
	{DESTROYER,            SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            FRIGATE             },
	{DESTROYER|SHIP_END,   SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            FRIGATE|SHIP_END    },
	{SEA,                  SEA,                            CORVETTE|SHIP_END,              SEA,                SEA,                SUBMARINE|SHIP_END,             SEA,                            SEA                 },
	{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 },
	{SEA,                  CARRIER|HORIZONTAL|SHIP_END,    CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL, CARRIER|HORIZONTAL, CARRIER|HORIZONTAL,             CARRIER|HORIZONTAL|SHIP_END,    SEA                 },
	{SEA,                  SEA,                            SEA,                            SEA,                SEA,                SEA,                            SEA,                            SEA                 }};
	**/
}

// reveals all the computer's ships
void reveal_computer_ships() {
	for (uint8_t y = 0; y < GRID_NUM_ROWS; y++) {
		for (uint8_t x = 0; x < GRID_NUM_COLUMNS; x++) {
			uint8_t cell = computer_grid[y][x];
			if ((cell & SHIP_MASK) && !(cell & (HIT_MASK | MISS_MASK | SUNK_MASK))) {
				ledmatrix_draw_pixel_in_computer_grid(x, y, COLOUR_ORANGE);
			}
		}
	}
}

// Hides the location of the computer's ships
void hide_computer_ships() {
	for (uint8_t y = 0; y < GRID_NUM_ROWS; y++) {
		for (uint8_t x = 0; x < GRID_NUM_COLUMNS; x++) {
			uint8_t cell = computer_grid[y][x];
			if ((cell & SHIP_MASK) && !(cell & (HIT_MASK | MISS_MASK | SUNK_MASK))) {
				ledmatrix_draw_pixel_in_computer_grid(x, y, COLOUR_BLACK);
			}
		}
	}
}