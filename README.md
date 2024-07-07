# AVR Microcontroller Project: Battleship Game

## Overview

This project is a replica of the classic game "Battleship" implemented on an AVR ATmega324A microcontroller. The game features interaction via push buttons, LED matrix display, and additional devices like a seven-segment display. 

## Features

- **Splash Screen**: 
  - Displays a welcome message with the player's name and student number.
  - Instructions on how to start the game using push buttons or terminal input.

- **Cursor Movement**: 
  - Use push buttons (B0, B1, B2, B3) to move the cursor up, down, left, and right.
  - Terminal input (W, A, S, D) also moves the cursor.

- **Human Turn**: 
  - Fire shots at the computer player's board by pressing a designated button or terminal key (F).
  - Visual feedback on hits (red) and misses (green) is displayed on the LED matrix.

- **Computer Turn**: 
  - The computer player fires at the human player's board in a basic or advanced (search & destroy) pattern.
  - Shots are marked on the LED matrix to show hits and misses.

- **Game Over**: 
  - When all ships of one player are sunk, the game announces the winner.
  - The LED matrix shows the final board state, and the game can be reset for a new round.

- **Additional Features**:
  - **Sound Effects**: Audio feedback for various game events, such as hits, sinks, and game over, using a buzzer.
  - **Game Pause**: Ability to pause and resume the game, with a message displayed on the terminal.
  - **Cheating Mode**: Allows the human player to reveal the computer's ships briefly or fire multiple shots in one turn.

## Components

- **ATmega324A Microcontroller**: 
  - Central processing unit for the game logic and hardware interfacing.
  - Handles input from buttons, manages the LED matrix display, and communicates via the serial terminal.

- **LED Matrix Display**: 
  - A visual display for the game board, showing both the human and computer player's grids.
  - Lights up different colors to indicate ship positions, hits, misses, and cursor movement.

- **Push Buttons**: 
  - Four buttons (B0, B1, B2, B3) connected to the microcontroller for moving the cursor and interacting with the game.
  - Each button corresponds to a direction or action (e.g., firing a shot).

- **Serial Terminal**: 
  - Provides additional game interaction through terminal input.
  - Displays messages, game status, and user instructions.

- **Buzzer**: 
  - Generates sound effects for game events like hits, ship sinks, and game over.
  - Adds an audio dimension to the game, enhancing the user experience.

- **Wiring and Connectivity**: 
  - Proper wiring and connections are essential for the project to function correctly.
  - Includes connections for buttons, LED matrix, seven-segment display, and other peripherals as outlined in the project specification.

## Project Structure

- **project.c**: Main game loop and event handling.
- **game.c/.h**: Game logic and state management.
- **display.c/.h**: Functions for displaying the game state on the LED matrix.
- **buttons.c/.h**: Handles push button inputs.
- **serialio.c/.h**: Manages serial communication for terminal input and output.
- **timer0.c/.h**: Sets up a timer for precise game event timing.

## Installation and Usage
- **Build the Project**: Use AVR-GCC or Microchip Studio to compile the code.
- **Upload to Microcontroller**: Use an AVR programmer to upload the compiled code to the ATmega324A microcontroller.
- **Connect the Hardware**: Assemble the circuit as per the wiring instructions in the project-specification file. Polulu was also used to connecty the microntroller to the computer via USB.
- **Play the Game**: Use the push buttons and terminal to interact with the game.

# Game of Life Simulation

## Overview

This project involves creating a simulation of John Conway's "Game of Life" (GoL) using Python. The Game of Life is a cellular automaton devised by mathematician John Conway. It consists of a grid of cells that evolve through iterations according to simple rules based on the states of neighboring cells. 

## Features

### Game of Life Simulation

## Components

- **Python Scripts**:
  - `conway.py`: Core module containing the implementation of the Game of Life rules and pattern importation methods.
  - `rle.py`: Module for reading and handling run length encoded (RLE) patterns.
  - `test_gameoflife_glider_simple.py`: Script for testing basic GoL patterns with simple animation.
  - `test_gameoflife_glider.py`: Script for testing GoL patterns with more complex animation.
  - `test_gameoflife_turing.py`: Script for demonstrating the Turing completeness of GoL.

- **Libraries Used**:
  - `numpy`: For efficient numerical computations.
  - `scipy`: For scientific and technical computing.
  - `matplotlib`: For creating animations and visualizing the evolution of GoL patterns.
