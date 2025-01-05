#include <FastLED.h>

// Hardware settings
#define PLAYER1_PIN 2  // Pin connected to Player 1 button
#define PLAYER2_PIN 3  // Pin connected to Player 2 button
#define LED_PIN 4      // Pin connected to the LED strip

// LED strip settings
#define NUM_LEDS 144        // Number of LEDs in the strip
#define LEDS_BRIGHTNESS 20  // Brightness of the LEDs (0-255)

// Game settings
#define RACKET_LENGTH 15           // Length of the Pong racket
#define VX_BASE 0.4                // Base speed of the Pong ball
#define SPEED_BALL_MIN 0.15        // Minimum speed of the Pong ball
#define SPEED_BALL_MAX 2           // Maximum speed of the Pong ball
#define SPEED_BALL_INCREMENT 0.1  // Increment of the Pong ball speed at each hit
#define PLAYER_DISABLED_DELAY 500  // Delay in ms during which a player's button is disabled after hitting the ball
#define VICTORY_DELAY 3000         // Delay in ms for victory animation
#define DEFEAT_DELAY 2000          // Delay in ms for defeat animation
#define SCORE_MAX 5                // Maximum score to win a game
#define ANIMATION_SPEED 75         // Speed of animations in the choice menu (lower is faster)
#define CTF_BASE_SPEED 40         // Initial speed of the ships (delay in ms between movements)
#define CTF_SPEED_INCREMENT 8     // Amount of speed increase (decrease in delay) when a point is captured

CRGB leds[NUM_LEDS];  // Array to store LED colors

// Game modes
enum GameMode {
  CHOICE,     // Game selection menu
  PONG,       // Pong game
  PONGSPEED,  // Pong game with increasing speed
  REFLEXE,    // Reflex game
  PUSH,       // Push game
  CTF         // Capture The Flag game
};

GameMode gameMode = CHOICE;  // Current game mode
int selectedGame = 0;       // Index of the currently selected game in the choice menu (0: Pong, 1: PongSpeed, 2: Reflexe, 3: Push, 4: CTF)

bool gameSetup = false;  // Flag to indicate if the current game has been set up

// Structure to represent the Pong ball
struct Ball {
  float x;     // X position of the ball (can be fractional for smoother movement)
  int dx;      // Direction of the ball along the X axis (1 or -1)
  float vx;    // Velocity of the ball along the X axis
  CRGB color;  // Color of the ball
};

// Structure to represent a player
struct Player {
  CRGB color;                     // Main color of the player
  CRGB colorDisabled;             // Color when the player is disabled
  CRGB raketColor1;               // First color of the racket gradient
  CRGB raketColor2;               // Second color of the racket gradient
  int score;                      // Player's score
  unsigned long disabledEndTime;  // Time at which the player's button will be re-enabled
  bool press;                     // Flag to indicate if the player's button is currently pressed
};

// Initialize the Pong ball
Ball ball = {
  .x = NUM_LEDS / 2,    // Start at the center
  .dx = 1,              // Start moving to the right
  .vx = VX_BASE,        // Initial speed
  .color = CRGB::White  // White color
};

// Initialize Player 1
Player player1 = {
  .color = CRGB::Blue,            // Blue
  .colorDisabled = CRGB::Orange,  // Orange
  .raketColor1 = CRGB::Blue,
  .raketColor2 = CRGB::DarkTurquoise,
  .score = 0,            // Initial score is 0
  .disabledEndTime = 0,  // Button is initially enabled
  .press = false         // Button is not initially pressed
};

// Initialize Player 2
Player player2 = {
  .color = CRGB::Red,             // Red
  .colorDisabled = CRGB::Purple,  // Purple
  .raketColor1 = CRGB::DarkOrange,
  .raketColor2 = CRGB::DarkRed,
  .score = 0,            // Initial score is 0
  .disabledEndTime = 0,  // Button is initially enabled
  .press = false         // Button is not initially pressed
};

// Structure to represent a ship in Capture the Flag
struct Ship {
  int position;
  int direction;  // 1 for right, -1 for left
  int speed;      // Delay between movements (lower is faster)
  unsigned long lastMoveTime;
  CRGB color;
};

Ship ship1 = {
  .position = 0,
  .direction = 1,
  .speed = CTF_BASE_SPEED,
  .lastMoveTime = 0,
  .color = CRGB::Blue
};

Ship ship2 = {
  .position = NUM_LEDS - 1,
  .direction = -1,
  .speed = CTF_BASE_SPEED,
  .lastMoveTime = 0,
  .color = CRGB::Red
};

// Setup function (called once at the beginning)
void setup() {
  Serial.begin(9600);  // Initialize serial communication for debugging

  // Initialize the LED strip
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LEDS_BRIGHTNESS);
  FastLED.clear();  // Turn off all LEDs

  // Set the player button pins as inputs with pull-up resistors
  pinMode(PLAYER1_PIN, INPUT_PULLUP);
  pinMode(PLAYER2_PIN, INPUT_PULLUP);

  on();  // animation at start
}

// animation at start
void on() {
  unsigned long loopStart = millis();
  while (millis() - loopStart < VICTORY_DELAY) {
    fill_rainbow(leds, NUM_LEDS, (millis() - loopStart) / 10, 1);
    FastLED.show();
  }
}

// Main loop function (called repeatedly)
void loop() {
  switch (gameMode) {
    case CHOICE:
      choiceLoop();  // Handle game selection
      break;
    case PONG:
      pongLoop();  // Run the Pong game
      break;
    case PONGSPEED:
      pongSpeedLoop(); // Run the PongSpeed game
      break;
    case REFLEXE:
      reflexeLoop();  // Run the Reflex game
      break;
    case PUSH:
      pushLoop();  // Run the Push game
      break;
    case CTF:
      ctfLoop();  // Run the Push game
      break;
  }
}

// ---------------------- CHOICE -----------------------------

// Function to handle the game selection menu
void choiceLoop() {
  if (!gameSetup) {
    FastLED.clear();   // Turn off all LEDs
    gameSetup = true;  // Set the setup flag
  }

  // Handle input from Player 1's button (change game selection)
  if (digitalRead(PLAYER1_PIN) == LOW) {
    handleButtonPress(PLAYER1_PIN);
    selectedGame = (selectedGame + 1) % 5;  // Cycle through 0, 1, 2, 3, 4
  }

  // Handle input from Player 2's button (confirm selection)
  if (digitalRead(PLAYER2_PIN) == LOW) {
    handleButtonPress(PLAYER2_PIN);
    selectGame();
  }

  // Display the animation for the currently selected game
  animateChoice();
}

// Function to handle button presses with debouncing
void handleButtonPress(int buttonPin) {
  delay(200);  // Debounce delay to filter out spurious signals
  while (digitalRead(buttonPin) == LOW)
    ;  // Wait for the button to be released
}

// Function to select and start the chosen game
void selectGame() {
  switch (selectedGame) {
    case 0:
      gameMode = PONG;
      break;
    case 1:
      gameMode = PONGSPEED;
      break;
    case 2:
      gameMode = REFLEXE;
      break;
    case 3:
      gameMode = PUSH;
      break;
    case 4:
      gameMode = CTF;
      break;
  }
  gameSetup = false;  // Reset setup flag for the selected game
  FastLED.clear();    // Clear LEDs
  FastLED.show();     // Update LEDs
}

// Function to display animations in the choice menu
void animateChoice() {
  FastLED.clear();                            // Turn off all LEDs
  int animationOffset = (NUM_LEDS / 2) - 15;  // Center the animation

  // Select the animation based on selectedGame
  switch (selectedGame) {
    case 0:  // Pong
      animatePong(animationOffset, 30);
      break;
    case 1:  // Pong Speed
      animatePongSpeed(animationOffset, 30);
      break;
    case 2:  // Reflexe
      animateReflexe(animationOffset, 30);
      break;
    case 3:  // Push
      animatePush(animationOffset, 30);
      break;
    case 4:  // Capture the Flag
      animateCaptureTheFlag(animationOffset, 30);
      break;
  }

  FastLED.show();          // Update LEDs
  delay(ANIMATION_SPEED);  // Control animation speed
}

// Function to animate the Pong game in the choice menu
void animatePong(int offset, int length) {
  static int pongBallX = 0;  // Current X position of the "ball"
  static int pongDir = 1;    // Current direction of the "ball"

  // Draw "rackets"
  leds[offset] = CRGB::Blue;
  leds[offset + length - 1] = CRGB::Red;

  // Move the "ball"
  pongBallX += pongDir;
  if (pongBallX <= 0 || pongBallX >= length - 1) {
    pongDir *= -1;  // Reverse direction when hitting an edge
  }

  leds[offset + pongBallX] = CRGB::White;
}

// Function to animate the PongSpeed game in the choice menu
void animatePongSpeed(int offset, int length) {
  static int pongSpeedBallX = 0;  // Current X position of the "ball"
  static int pongSpeedDir = 1;    // Current direction of the "ball"
  static int pongSpeedVX = 2;

  // Draw "rackets"
  leds[offset] = CRGB::Blue;
  leds[offset + length - 1] = CRGB::Red;

  // Move the "ball"
  pongSpeedBallX += pongSpeedDir * pongSpeedVX;
  if (pongSpeedBallX <= 0 || pongSpeedBallX >= length - 2) {
    pongSpeedDir *= -1;  // Reverse direction when hitting an edge
  }

  leds[offset + pongSpeedBallX] = CRGB::White;
}

// Function to animate the Reflex game in the choice menu
void animateReflexe(int offset, int length) {
  static unsigned long lastChange = 0;  // Time of the last state change
  static bool reflexOn = false;         // Current state of the animation

  if (millis() - lastChange > 300) {
    reflexOn = !reflexOn;   // Toggle the state
    lastChange = millis();  // Update the last change time
  }

  if (reflexOn) {
    fill_solid(leds + offset, length, CRGB::Green);  // Fill with green when on
  }
}

// Function to animate the Push game in the choice menu
void animatePush(int offset, int length) {
  static int pushBallX = 15;  // Current X position of the "ball"
  static int pushDir = 1;     // Current direction of the "ball"

  // Move the "ball" towards a random direction
  if (random(0, 10) > 5) {
    pushDir *= -1;
  }

  pushBallX += pushDir;  // Move the ball

  if (pushBallX <= 0 || pushBallX >= length - 1) {
    pushBallX = length / 2;  // Reset to center if it hits an edge
  }

  leds[offset + pushBallX] = CRGB::Purple;
}

void animateCaptureTheFlag(int offset, int length) {
  unsigned long lastMoveTime = 0;
  int ship1X = 0;
  int ship2X = length - 1;
  int ship1Dir = 1;
  int ship2Dir = -1;

  if (millis() - lastMoveTime > 150) {
    lastMoveTime = millis();

    // Move ship 1
    leds[offset + ship1X] = CRGB::Black;
    ship1X += ship1Dir;
    if (ship1X >= length) {
      ship1X = length - 2;
      ship1Dir = -1;
    } else if (ship1X < 0) {
      ship1X = 1;
      ship1Dir = 1;
    }
    leds[offset + ship1X] = CRGB::Blue;

    // Move ship 2
    leds[offset + ship2X] = CRGB::Black;
    ship2X += ship2Dir;
    if (ship2X >= length) {
      ship2X = length - 2;
      ship2Dir = -1;
    } else if (ship2X < 0) {
      ship2X = 1;
      ship2Dir = 1;
    }
    leds[offset + ship2X] = CRGB::Red;

    // Draw a flag in the middle (you can make it move randomly for a more dynamic effect)
    leds[offset + length / 2] = CRGB::Yellow;
  }
}

// ---------------------- PONG GAME -----------------------------

// Function to run the Pong game loop
void pongLoop() {
  if (!gameSetup) {
    pongSetup();  // Initialize the Pong game
  }

  FastLED.clear();  // Clear the LED strip

  // Handle player input and update racket positions
  handlePlayerInput();

  // Display game elements
  displayGameElements();

  // Update ball position and check for collisions
  updateBallPosition();

  FastLED.show();  // Update the LED strip
}

// Function to handle player input and update racket positions
void handlePlayerInput() {
  // Handle input from Player 1
  if (digitalRead(PLAYER1_PIN) == LOW && millis() > player1.disabledEndTime && !player1.press) {
    player1.press = true;
    if (ball.x >= RACKET_LENGTH) {
      player1.disabledEndTime = millis() + PLAYER_DISABLED_DELAY;
    } else {
      ball.vx = calculateBallSpeed(RACKET_LENGTH - ball.x);
      ball.dx = 1;
      ball.color = player1.color;
    }
  }

  // Handle input from Player 2
  if (digitalRead(PLAYER2_PIN) == LOW && millis() > player2.disabledEndTime && !player2.press) {
    player2.press = true;
    if (ball.x < NUM_LEDS - RACKET_LENGTH) {
      player2.disabledEndTime = millis() + PLAYER_DISABLED_DELAY;
    } else {
      ball.vx = calculateBallSpeed(RACKET_LENGTH - (NUM_LEDS - ball.x));
      ball.dx = -1;
      ball.color = player2.color;
    }
  }

  // Reset button press flags when buttons are released
  if (digitalRead(PLAYER1_PIN) == HIGH) {
    player1.press = false;
  }
  if (digitalRead(PLAYER2_PIN) == HIGH) {
    player2.press = false;
  }
}

// Function to display game elements on the LED strip
void displayGameElements() {
  // Display Player 1's racket if not disabled
  if (millis() > player1.disabledEndTime) {
    fill_gradient_RGB(leds, 0, player1.raketColor1, RACKET_LENGTH - 1, player1.raketColor2);
  }

  // Display Player 2's racket if not disabled
  if (millis() > player2.disabledEndTime) {
    fill_gradient_RGB(leds, NUM_LEDS - RACKET_LENGTH, player2.raketColor1, NUM_LEDS, player2.raketColor2);
  }

  // Display other game elements (ball, scores, disabled rackets)
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == (int)ball.x) {
      leds[i] = ball.color;  // Draw the ball
    } else if (i < NUM_LEDS / 2 && i >= NUM_LEDS / 2 - player1.score) {
      leds[i] = CRGB::Green;  // Draw Player 1's score
    } else if (i >= NUM_LEDS / 2 && i < NUM_LEDS / 2 + player2.score) {
      leds[i] = CRGB::Yellow;  // Draw Player 2's score
    } else if (i < RACKET_LENGTH && millis() < player1.disabledEndTime) {
      leds[i] = player1.colorDisabled;  // Draw Player 1's disabled racket
    } else if (i >= NUM_LEDS - RACKET_LENGTH && millis() < player2.disabledEndTime) {
      leds[i] = player2.colorDisabled;  // Draw Player 2's disabled racket
    }
  }
}

// Function to update ball position and handle collisions
void updateBallPosition() {
  // Check for collisions with the edges
  if (ball.x <= 0 || ball.x >= NUM_LEDS) {
    if (ball.x >= NUM_LEDS) {
      player1Win();  // Player 1 scores
    } else {
      player2Win();  // Player 2 scores
    }
  }

  // Move the ball
  ball.x += ball.dx * ball.vx;
}

// Function to calculate the ball speed based on the distance from the racket edge
float calculateBallSpeed(float distance) {
  float speed = distance * distance / 100;
  return constrain(speed, SPEED_BALL_MIN, SPEED_BALL_MAX);
}

// Function to reset the ball to the center after a point is scored
void resetSet() {
  ball.x = RACKET_LENGTH;
  ball.color = player1.color;

  if (ball.dx == 1) {
    ball.x = NUM_LEDS - RACKET_LENGTH - 1;
    ball.color = player2.color;
  }

  ball.vx = VX_BASE;  // Reset to base speed
  ball.dx *= -1;      // Reverse direction

  // Check if a player has reached the maximum score
  if (player1.score >= SCORE_MAX || player2.score >= SCORE_MAX) {
    gameSetup = false;  // Reset game setup to start a new game
  }
}

// Function to set up the Pong game
void pongSetup() {
  // Check if a player has won the previous game
  if (player1.score >= SCORE_MAX) {
    unsigned long loopStart = millis();
    // Victory animation for player 1
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds, NUM_LEDS / 2, (millis() - loopStart));
      FastLED.show();
    }
  } else if (player2.score >= SCORE_MAX) {
    unsigned long loopStart = millis();
    // Victory animation for player 2
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds + NUM_LEDS / 2, NUM_LEDS / 2, (millis() - loopStart));
      FastLED.show();
    }
  } else {
    // Start direction
    ball.x = 1;
    ball.dx = 1;
    if (rand() % 2 == 1) {
      ball.x = NUM_LEDS - 1;
      ball.dx = -1;
    }
  }

  // Reset scores
  player1.score = 0;
  player2.score = 0;

  // Display countdown animation
  FastLED.clear();
  fill_solid(leds + NUM_LEDS / 2 - 2, 4, CRGB::Red);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 4, 8, CRGB::Yellow);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 6, 12, CRGB::Green);
  FastLED.show();
  delay(600);

  gameSetup = true;  // Set the setup flag
}

// Function to handle player 1 scoring a point
void player1Win() {
  player1.score++;  // Increment player 1's score
  // Point scored animation
  fill_rainbow_circular(leds + NUM_LEDS / 3, NUM_LEDS / 6, 100, false);
  FastLED.show();
  delay(1000);
  resetSet();
}

// Function to handle player 2 scoring a point
void player2Win() {
  player2.score++;  // Increment player 2's score
  // Point scored animation
  fill_rainbow_circular(leds + NUM_LEDS / 2, NUM_LEDS / 6, 100, true);
  FastLED.show();
  delay(1000);
  resetSet();
}

// ---------------------- PONGSPEED GAME -----------------------------

// Function to run the PongSpeed game loop
void pongSpeedLoop() {
  if (!gameSetup) {
    pongSpeedSetup();  // Initialize the PongSpeed game
  }

  FastLED.clear();  // Clear the LED strip

  // Handle player input and update racket positions
  handlePlayerInputPongSpeed();

  // Display game elements
  displayGameElementsPongSpeed();

  // Update ball position and check for collisions
  updateBallPositionPongSpeed();

  FastLED.show();  // Update the LED strip
}

// Function to handle player input for the PongSpeed game
void handlePlayerInputPongSpeed() {
  // Handle input from Player 1
  if (digitalRead(PLAYER1_PIN) == LOW && millis() > player1.disabledEndTime && !player1.press) {
    player1.press = true;
    if (ball.x >= RACKET_LENGTH) {
      player1.disabledEndTime = millis() + PLAYER_DISABLED_DELAY;
    } else {
      // Increase ball speed on each hit
      ball.vx += SPEED_BALL_INCREMENT;
      // Limit ball speed to the maximum speed
      ball.vx = constrain(ball.vx, VX_BASE, SPEED_BALL_MAX);
      ball.dx = 1;
      ball.color = player1.color;
    }
  }

  // Handle input from Player 2
  if (digitalRead(PLAYER2_PIN) == LOW && millis() > player2.disabledEndTime && !player2.press) {
    player2.press = true;
    if (ball.x < NUM_LEDS - RACKET_LENGTH) {
      player2.disabledEndTime = millis() + PLAYER_DISABLED_DELAY;
    } else {
      // Increase ball speed on each hit
      ball.vx += SPEED_BALL_INCREMENT;
      // Limit ball speed to the maximum speed
      ball.vx = constrain(ball.vx, VX_BASE, SPEED_BALL_MAX);
      ball.dx = -1;
      ball.color = player2.color;
    }
  }

  // Reset button press flags when buttons are released
  if (digitalRead(PLAYER1_PIN) == HIGH) {
    player1.press = false;
  }
  if (digitalRead(PLAYER2_PIN) == HIGH) {
    player2.press = false;
  }
}

// Function to display game elements for the PongSpeed game
void displayGameElementsPongSpeed() {
  // Display Player 1's racket if not disabled
  if (millis() > player1.disabledEndTime) {
    fill_gradient_RGB(leds, 0, player1.raketColor1, RACKET_LENGTH - 1, player1.raketColor2);
  }

  // Display Player 2's racket if not disabled
  if (millis() > player2.disabledEndTime) {
    fill_gradient_RGB(leds, NUM_LEDS - RACKET_LENGTH, player2.raketColor1, NUM_LEDS, player2.raketColor2);
  }

  // Display other game elements (ball, scores, disabled rackets)
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == (int)ball.x) {
      leds[i] = ball.color;  // Draw the ball
    } else if (i < NUM_LEDS / 2 && i >= NUM_LEDS / 2 - player1.score) {
      leds[i] = CRGB::Green;  // Draw Player 1's score
    } else if (i >= NUM_LEDS / 2 && i < NUM_LEDS / 2 + player2.score) {
      leds[i] = CRGB::Yellow;  // Draw Player 2's score
    } else if (i < RACKET_LENGTH && millis() < player1.disabledEndTime) {
      leds[i] = player1.colorDisabled;  // Draw Player 1's disabled racket
    } else if (i >= NUM_LEDS - RACKET_LENGTH && millis() < player2.disabledEndTime) {
      leds[i] = player2.colorDisabled;  // Draw Player 2's disabled racket
    }
  }
}

// Function to update ball position for the PongSpeed game
void updateBallPositionPongSpeed() {
  // Check for collisions with the edges
  if (ball.x <= 0 || ball.x >= NUM_LEDS) {
    if (ball.x >= NUM_LEDS) {
      player1WinPongSpeed();  // Player 1 scores
    } else {
      player2WinPongSpeed();  // Player 2 scores
    }
  }

  // Move the ball
  ball.x += ball.dx * ball.vx;
}

// Function to reset the ball for the PongSpeed game
void resetSetPongSpeed() {
  ball.x = RACKET_LENGTH;
  ball.color = player1.color;

  if (ball.dx == 1) {
    ball.x = NUM_LEDS - RACKET_LENGTH - 1;
    ball.color = player2.color;
  }

  ball.vx = VX_BASE;  // Reset to base speed
  ball.dx *= -1;      // Reverse direction

  // Check if a player has reached the maximum score
  if (player1.score >= SCORE_MAX || player2.score >= SCORE_MAX) {
    gameSetup = false;  // Reset game setup to start a new game
  }
}

// Function to set up the PongSpeed game
void pongSpeedSetup() {
  // Check if a player has won the previous game
  if (player1.score >= SCORE_MAX) {
    unsigned long loopStart = millis();
    // Victory animation for player 1
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds, NUM_LEDS / 2, (millis() - loopStart));
      FastLED.show();
    }
  } else if (player2.score >= SCORE_MAX) {
    unsigned long loopStart = millis();
    // Victory animation for player 2
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds + NUM_LEDS / 2, NUM_LEDS / 2, (millis() - loopStart));
      FastLED.show();
    }
  } else {
    // Start direction
    ball.x = 1;
    ball.dx = 1;
    if (rand() % 2 == 1) {
      ball.x = NUM_LEDS - 1;
      ball.dx = -1;
    }
  }

  // Reset scores
  player1.score = 0;
  player2.score = 0;

  // Reset speed
  ball.vx = VX_BASE;

  // Display countdown animation
  FastLED.clear();
  fill_solid(leds + NUM_LEDS / 2 - 2, 4, CRGB::Red);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 4, 8, CRGB::Yellow);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 6, 12, CRGB::Green);
  FastLED.show();
  delay(600);

  gameSetup = true;  // Set the setup flag
}

// Function to handle player 1 scoring a point in PongSpeed game
void player1WinPongSpeed() {
  player1.score++;  // Increment player 1's score
  // Point scored animation
  fill_rainbow_circular(leds + NUM_LEDS / 3, NUM_LEDS / 6, 100, false);
  FastLED.show();
  delay(1000);
  resetSetPongSpeed();
}

// Function to handle player 2 scoring a point in PongSpeed game
void player2WinPongSpeed() {
  player2.score++;  // Increment player 2's score
  // Point scored animation
  fill_rainbow_circular(leds + NUM_LEDS / 2, NUM_LEDS / 6, 100, true);
  FastLED.show();
  delay(1000);
  resetSetPongSpeed();
}

// ---------------------- REFLEXE GAME -----------------------------

unsigned long start = 0;  // Variable to store the start time of the Reflex game

// Function to run the Reflex game loop
void reflexeLoop() {
  if (!gameSetup) {
    reflexSetup();  // Initialize the Reflex game
  }

  unsigned long delaiReaction = max(9, millis() - start);  // Calculate the player's reaction time

  // Check if the reaction time exceeds the maximum allowed time
  if ((delaiReaction / 3 >= NUM_LEDS)) {
    reflexDefeat();     // Player loses
    gameSetup = false;  // Reset setup flag
    return;             // Exit the loop
  }

  // Handle player 1's input
  if (digitalRead(PLAYER1_PIN) == LOW) {
    FastLED.clear();  // Turn off all LEDs

    // Display player 1's reaction time
    fill_solid(leds, delaiReaction / 3, player1.color);
    FastLED.show();

    delay(VICTORY_DELAY);        // Wait for 5 seconds
    gameSetup = false;  // Reset setup flag
    return;             // Exit the loop
  }

  // Handle player 2's input
  if (digitalRead(PLAYER2_PIN) == LOW) {
    FastLED.clear();  // Turn off all LEDs

    // Display player 2's reaction time
    fill_solid(leds + NUM_LEDS - delaiReaction / 3, delaiReaction / 3, player2.color);
    FastLED.show();

    delay(VICTORY_DELAY);        // Wait for 5 seconds
    gameSetup = false;  // Reset setup flag
    return;             // Exit the loop
  }
}

// Function to set up the Reflex game
void reflexSetup() {
  FastLED.clear();  // Turn off all LEDs

  // Display countdown animation
  fill_solid(leds + NUM_LEDS / 2 - 2, 4, CRGB::Blue);
  FastLED.show();
  delay(300);
  fill_solid(leds + NUM_LEDS / 2 - 4, 8, CRGB::Red);
  FastLED.show();
  delay(300);
  fill_solid(leds + NUM_LEDS / 2 - 6, 12, CRGB::Yellow);
  FastLED.show();
  delay(300);
  fill_solid(leds + NUM_LEDS / 2 - 8, 16, CRGB::Green);
  FastLED.show();

  int wait = rand() % 5000;
  Serial.println(wait);
  delay(wait);

  FastLED.clear();  // Turn off all LEDs
  FastLED.show();


  if (digitalRead(PLAYER1_PIN) == LOW || digitalRead(PLAYER2_PIN) == LOW) {
    reflexToEarly();     // Player loses
    gameSetup = false;  // Reset setup flag
    return;             // Exit the loop
  }

  start = millis();  // Record the start time
  gameSetup = true;  // Set the setup flag
}

// Function to handle player defeat in the Reflex game
void reflexDefeat() {
  unsigned long loopStart = millis();
  bool reversed = false;
  // Defeat animation
  while (millis() - loopStart < DEFEAT_DELAY) {
    reversed = true;
    if ((millis() - loopStart) / 300 % 2 == 1) {
      reversed = false;
    }
    fill_rainbow_circular(leds + 30, NUM_LEDS - 60, (millis() - loopStart), reversed);
    FastLED.show();
  }
}

// Function to handle player defeat in the Reflex game
void reflexToEarly() {
  unsigned long loopStart = millis();
  bool reversed = false;
  // Defeat animation
  while (millis() - loopStart < DEFEAT_DELAY) {
    fill_solid(leds + 30, NUM_LEDS - 60, CRGB::DarkRed);
    FastLED.show();
  }
}

// ---------------------- Push GAME -----------------------------

// Function to run the Push game loop
void pushLoop() {
  if (!gameSetup) {
    pushSetup();  // Initialize the Push game
  }

  // Handle player input and update ball position
  handlePushGameInput();

  // Display game elements
  displayPushGameElements();

  // Check for win conditions and handle accordingly
  checkPushGameWinConditions();
}

// Function to handle player input for the Push game
void handlePushGameInput() {
  // Handle input from Player 1
  if (digitalRead(PLAYER1_PIN) == LOW && !player1.press) {
    player1.press = true;
    ball.x += 5;  // Move the ball to the right
  }

  // Handle input from Player 2
  if (digitalRead(PLAYER2_PIN) == LOW && !player2.press) {
    player2.press = true;
    ball.x -= 5;  // Move the ball to the left
  }

  // Reset button press flags when buttons are released
  if (digitalRead(PLAYER1_PIN) == HIGH) {
    player1.press = false;
  }
  if (digitalRead(PLAYER2_PIN) == HIGH) {
    player2.press = false;
  }
}

// Function to display Push game elements on the LED strip
void displayPushGameElements() {
  FastLED.clear();                 // Turn off all LEDs
  leds[(int)ball.x] = ball.color;  // Draw the ball
  FastLED.show();                  // Update LEDs
}

// Function to check for win conditions in the Push game
void checkPushGameWinConditions() {
  if (ball.x <= 0) {
    player2WinLoop();  // Player 2 wins
  }
  if (ball.x >= NUM_LEDS) {
    player1WinLoop();  // Player 1 wins
  }
}

// Function to set up the Push game
void pushSetup() {
  ball.x = NUM_LEDS / 2;  // Reset ball position to the center

  // Display countdown animation
  FastLED.clear();
  fill_solid(leds + NUM_LEDS / 2 - 2, 4, CRGB::Red);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 4, 8, CRGB::Yellow);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 6, 12, CRGB::Green);
  FastLED.show();
  delay(600);

  gameSetup = true;       // Set the setup flag
}

// Function to handle player 1 winning a round in the Push game
void player1WinLoop() {
  // Victory animation for player 1
  fill_rainbow_circular(leds, NUM_LEDS, 0, false);
  FastLED.show();
  FastLED.clear();
  unsigned long loopStart = millis();
  while (millis() - loopStart < VICTORY_DELAY) {
    fill_rainbow_circular(leds, NUM_LEDS / 6, (millis() - loopStart), false);
    FastLED.show();
  }
  gameSetup = false;       // Set the setup flag
}

// Function to handle player 2 winning a round in the Push game
void player2WinLoop() {
  // Victory animation for player 2
  fill_rainbow_circular(leds, NUM_LEDS, 0, true);
  FastLED.show();
  FastLED.clear();
  unsigned long loopStart = millis();
  while (millis() - loopStart < VICTORY_DELAY) {
    fill_rainbow_circular(leds + NUM_LEDS - NUM_LEDS / 6, NUM_LEDS / 6, (millis() - loopStart), true);
    FastLED.show();
  }
  gameSetup = false;       // Set the setup flag
}

// ---------------------- CAPTURE THE FLAG GAME -----------------------------

int flagPosition;
bool flagCaptured = false;

void ctfLoop() {
  if (!gameSetup) {
    ctfSetup();

  }

  handleCtfInput();
  moveShips();
  checkFlagCapture();
  displayCtfGameElements();

  if (player1.score >= SCORE_MAX || player2.score >= SCORE_MAX) {
    ctfGameOver(player1.score >= SCORE_MAX ? 1 : 2);
  }
}

void ctfSetup() {
  // Initialize ships
  ship1.position = 0;
  ship1.direction = 1;
  ship1.speed = CTF_BASE_SPEED;
  ship1.lastMoveTime = 0;

  ship2.position = NUM_LEDS - 1;
  ship2.direction = -1;
  ship2.speed = CTF_BASE_SPEED;
  ship2.lastMoveTime = 0;

  // Reset scores
  player1.score = 0;
  player2.score = 0;

  // Place the flag for the first time
  placeFlag();

  gameSetup = true;
}

void handleCtfInput() {
    // Handle input from Player 1
    if (digitalRead(PLAYER1_PIN) == LOW && !player1.press) {
        player1.press = true;
        // Check if ship1 is at, one position before, or one position after flagPosition
        if (ship1.position >= flagPosition - 1 && ship1.position <= flagPosition + 1) {
            player1.score++;
            flagCaptured = true;
        } else {
            player1.score = max(0, player1.score - 1); // Decrement score, but not below 0
        }
    }
    ship1.speed = CTF_BASE_SPEED - player1.score * CTF_SPEED_INCREMENT; // Increase speed

    // Handle input from Player 2
    if (digitalRead(PLAYER2_PIN) == LOW && !player2.press) {
        player2.press = true;
        // Check if ship2 is at, one position before, or one position after flagPosition
        if (ship2.position >= flagPosition - 1 && ship2.position <= flagPosition + 1) {
            player2.score++;
            flagCaptured = true;
        } else {
            player2.score = max(0, player2.score - 1); // Decrement score, but not below 0
        }
    }
    ship2.speed = CTF_BASE_SPEED - player2.score * CTF_SPEED_INCREMENT; // Increase speed

    // Reset button press flags when buttons are released
    if (digitalRead(PLAYER1_PIN) == HIGH) {
        player1.press = false;
    }
    if (digitalRead(PLAYER2_PIN) == HIGH) {
        player2.press = false;
    }
}

void moveShips() {
  unsigned long currentTime = millis();

  // Move ship 1
  if (currentTime - ship1.lastMoveTime > ship1.speed) {
    ship1.lastMoveTime = currentTime;
    leds[ship1.position] = CRGB::Black;  // Clear previous position
    ship1.position += ship1.direction;

    // Handle boundary conditions for ship 1
    if (ship1.position >= NUM_LEDS) {
      ship1.position = NUM_LEDS - 2;
      ship1.direction = -1;
    } else if (ship1.position < 0) {
      ship1.position = 1;
      ship1.direction = 1;
    }
  }

  // Move ship 2
  if (currentTime - ship2.lastMoveTime > ship2.speed) {
    ship2.lastMoveTime = currentTime;
    leds[ship2.position] = CRGB::Black;  // Clear previous position
    ship2.position += ship2.direction;

    // Handle boundary conditions for ship 2
    if (ship2.position >= NUM_LEDS) {
      ship2.position = NUM_LEDS - 2;
      ship2.direction = -1;
    } else if (ship2.position < 0) {
      ship2.position = 1;
      ship2.direction = 1;
    }
  }
}

void checkFlagCapture() {
  if (flagCaptured) {
    animateFlagCapture();
    placeFlag();
    flagCaptured = false;  // Reset the flag
  }
}

void animateFlagCapture() {
  for (int i = 0; i < 3; i++) {
    leds[flagPosition] = CRGB::Green;  // Highlight captured flag (green)
    FastLED.show();
    delay(200);
    leds[flagPosition] = CRGB::Yellow;  // Change back to flag color (yellow)
    FastLED.show();
    delay(200);
  }
}

void placeFlag() {
  do {
    flagPosition = random(NUM_LEDS);
  } while (flagPosition == ship1.position || flagPosition == ship2.position);  // Make sure the flag doesn't spawn on a ship
}

void displayCtfGameElements() {
    FastLED.clear();

    // Display scores
    for (int i = 0; i < player1.score; i++) {
        leds[i] = player1.color; // Score for player 1 on the left
    }
    for (int i = 0; i < player2.score; i++) {
        leds[NUM_LEDS - 1 - i] = player2.color; // Score for player 2 on the right
    }

    // Display ships
    leds[ship1.position] = ship1.color;
    leds[ship2.position] = ship2.color;

    // Display flag
    leds[flagPosition] = CRGB::Yellow; // Color of the flag

    FastLED.show();
}

void ctfGameOver(int winner) {
  // Display victory animation based on the winner
  CRGB winnerColor = (winner == 1) ? player1.color : player2.color;
  for (int i = 0; i < 3; i++) {
    fill_solid(leds, NUM_LEDS, winnerColor);
    FastLED.show();
    delay(500);
    FastLED.clear();
    FastLED.show();
    delay(500);
  }
  gameSetup = false;  // Reset setup flag
}
