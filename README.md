# 1D-Pong-Led
# Arduino 1D Pong Game
This project is an Arduino-based 1D Pong game using a strip of 144 LEDs connected to pin 4 and two buttons connected to pins 2 and 3.

# Dependencies
This project uses the FastLED library. You can install it from the Arduino IDE Library Manager or download it from GitHub.

# Hardware
Arduino board
144 LED strip (WS2812B)
2 push buttons
Setup
Connect the LED strip data pin to Arduino pin 4 and the buttons to pins 2 and 3.

# Game Modes
The game supports several modes:

Pong: Classic Pong game. Each player controls an end of the LED strip. The goal is to hit the ball (a white LED) towards the opponent.
Tug of War: Players press their buttons to move the ball towards the opponent's side.
Pong 2: A variant of the classic Pong game.

# Code Overview
The code is structured around several key functions:

setup(): Initializes the LED strip and buttons.
loop(): Main game loop, which calls the appropriate function based on the current game state.
start(): Prepares the game based on the selected game mode.
game(): Handles the game logic.
end(): Displays the winner and ends the game.
Each game mode has its own set of functions for handling game logic, player input, and display updates.

# Contributing
Contributions are welcome! Please read the contributing guide for more information.

# License
This project is licensed under the MIT License - see the LICENSE file for details.
