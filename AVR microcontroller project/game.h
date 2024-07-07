/*
 * game.h
 *
 * Author: Jarrod Bennett, Cody Burnett
 *
 * Function prototypes for game functions available externally. You may wish
 * to add extra function prototypes here to make other functions available to
 * other files.
 */


#ifndef GAME_H_
#define GAME_H_

#include <stdint.h>

// Initialize the game by resetting the grid and beat
void initialise_game(void);

// flash the cursor
void flash_cursor(void);

// move the cursor in the x and/or y direction
void move_cursor(int8_t dx, int8_t dy);
void invalid_move_message(void);
void reset_invalid_move(void);
void clear_invalid_move_message(void);
void sunk_ship_message(const char* player, const char* ship);
void add_adjacent_cells_to_hit(uint8_t x, uint8_t y);
void computer_fire_animation(uint8_t target_x, uint8_t target_y);
void computer_turn(void);
void fire_at_location(int8_t dx, int8_t dy);
void play_sound(const char *event);
void sound_on(uint16_t freq, float dutycycle, uint16_t clockperiod, uint16_t pulsewidth);
void sound_off();
void fire_around_location();
void fire_in_row();
void fire_in_column();
void reset_game();
void reveal_computer_ships();
void hide_computer_ships();

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void);

#define SEA 0
#define CARRIER 1
#define CRUISER 2
#define DESTROYER 3
#define FRIGATE 4
#define CORVETTE 5
#define SUBMARINE 6
#define SHIP_MASK 7
#define SHIP_END 8
#define HORIZONTAL 16

#endif
