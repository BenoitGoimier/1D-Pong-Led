#include <FastLED.h>

// setting hardware
#define PLAYER1_PIN 2
#define PLAYER2_PIN 3
#define LED_PIN 4

// setting led
#define NUM_LEDS 144
#define LEDS_BRIGHTNESS 20

// setting jeu
#define RACKET_LENGTH 15
#define VX_BASE 0.4
#define SPEED_BALL_MIN 0.15
#define SPEED_BALL_MAX 2
#define PLAYER_DISABLED_DELAY 500
#define VICTORY_DELAY 3000
#define SCORE_MAX 5

CRGB leds[NUM_LEDS];

enum GameMode {
  CHOICE,
  PONG,
  REFLEXE,
  PUSH
};

GameMode gameMode = PONG;

bool gameSetup = false;

struct Ball {
  float x;
  int dx;
  float vx;
  CRGB color;
};

struct Player {
  CRGB color;
  CRGB colorDisabled;
  int score;
  unsigned long disabledEndTime;
  bool press;
};

Ball ball = {
  .x = NUM_LEDS / 2,
  .dx = 1,
  .vx = VX_BASE,
  .color = CRGB::White
};

Player player1 = {
  .color = CRGB::Blue,
  .colorDisabled = CRGB::Orange,
  .score = 0,
};

Player player2 = {
  .color = CRGB::Red,
  .colorDisabled = CRGB::Purple,
  .score = 0,
};

void setup() {
  Serial.begin(9600);
  
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LEDS_BRIGHTNESS);
  
  FastLED.clear();

  pinMode(PLAYER1_PIN, INPUT_PULLUP);
  pinMode(PLAYER2_PIN, INPUT_PULLUP);

  on();
}

void on() {
    int loopStart = millis();
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds, NUM_LEDS, (millis() - loopStart) / 10, 1);
      FastLED.show();
    }
}

void loop() {
  switch (gameMode) {
    case CHOICE: 
      choiceLoop();
      break;
    case PONG: 
      pongLoop();
      break;
    case REFLEXE: 
      reflexeLoop();
      break;
    case PUSH: 
      pushLoop();
      break;
  }
}

// ---------------------- CHOICE-----------------------------

void choiceLoop() {

  // Rafraîchissement de l'affichage
  FastLED.clear();
  for (int i = NUM_LEDS; i < NUM_LEDS; i++) {
    if (i == (int)ball.x) {
      leds[i] = ball.color;
    } else if (i >= NUM_LEDS / 4 && i < NUM_LEDS / 4 + RACKET_LENGTH) {
      leds[i] = player1.color;
    } else if (i >= NUM_LEDS / 4 * 3 && i < NUM_LEDS / 4 * 3 + RACKET_LENGTH) {
      leds[i] = player2.color;
    }
  }
  
  if (ball.x < NUM_LEDS / 4 || ball.x > NUM_LEDS / 4 * 3) {
    ball.dx *= -1;
  }

  ball.x += ball.dx * ball.vx;
  FastLED.show();
}

// ---------------------- PONG GAME-----------------------------


void pongLoop() {
  if (!gameSetup) {
    pongSetup();
  }
  
  // Rafraîchissement de l'affichage
  FastLED.clear();

  if (millis() > player1.disabledEndTime) { // affichage de la raquette du P1
    fill_gradient_RGB(leds, 0, CRGB::Blue, RACKET_LENGTH -1, CRGB::DarkTurquoise);
  }
  if (millis() > player2.disabledEndTime) { // affichage de la raquette du P2
    fill_gradient_RGB(leds, NUM_LEDS - RACKET_LENGTH, CRGB::DarkOrange, NUM_LEDS, CRGB::DarkRed);
  }

  for (int i = 0; i < NUM_LEDS; i++) {
    if (i == (int)ball.x) { // affichage de la balle
      leds[i] = ball.color;
    } else if (i < NUM_LEDS / 2 && i >= NUM_LEDS / 2 - player1.score) { // affichage du score P1
      leds[i] = CRGB::Green;
    } else if (i >= NUM_LEDS / 2 && i < NUM_LEDS / 2 + player2.score) { // affichage du score P2
      leds[i] = CRGB::Yellow;
    } else if (i < RACKET_LENGTH && millis() < player1.disabledEndTime) { // affichage de la raquette désactivé du P1
      leds[i] = player1.colorDisabled;
    } else if (i >= NUM_LEDS - RACKET_LENGTH && millis() < player2.disabledEndTime) { // affichage de la raquette désactivé du P2
      leds[i] = player2.colorDisabled;
    }
  }

  // Gestion des interactions avec les joueurs
  if (digitalRead(PLAYER1_PIN) == LOW && millis() > player1.disabledEndTime && !player1.press) {
    player1.press = true;
    if (ball.x >= RACKET_LENGTH) {
      player1.disabledEndTime = millis() + PLAYER_DISABLED_DELAY;
    } else {
      ball.vx = ballSpeed(RACKET_LENGTH - ball.x);
      ball.dx = 1;
      ball.color = player1.color;
    }
  }

  if (digitalRead(PLAYER2_PIN) == LOW && millis() > player2.disabledEndTime && !player2.press) {
    player2.press = true;
    if (ball.x < NUM_LEDS - RACKET_LENGTH) {
      player2.disabledEndTime = millis() + PLAYER_DISABLED_DELAY;
    } else {
      ball.vx = ballSpeed(RACKET_LENGTH - (NUM_LEDS - ball.x));
      ball.dx = -1;
      ball.color = player2.color;
    }
  }

  // eviter le hack de rester appuyé sur le bouton
  if (digitalRead(PLAYER1_PIN) == HIGH) {
    player1.press = false;
  }

  if (digitalRead(PLAYER2_PIN) == HIGH) {
    player2.press = false;
  }

  FastLED.show();

  // Vérification des collisions
  if (ball.x < -1 || ball.x >= NUM_LEDS) {
    if (ball.x >= NUM_LEDS) {
      player1Win();
    } else {
      player2Win();
    }
  }

  // Déplacement de la balle
  ball.x += ball.dx * ball.vx;
}

float ballSpeed(float distance) {
  float speed = distance * distance / 100;
  speed = max(speed, SPEED_BALL_MIN);
  speed = min(speed, SPEED_BALL_MAX);

  return speed;
}

void resetSet() {
  ball.x = RACKET_LENGTH;
  ball.color = player1.color;
  if (ball.dx == 1) {
    ball.x = NUM_LEDS - RACKET_LENGTH -1;
    ball.color = player2.color;
  }
  ball.vx = VX_BASE;
  ball.dx *= -1;

  if (player1.score >= SCORE_MAX || player2.score >= SCORE_MAX) {
    gameSetup = false;
  }
}

void pongSetup() {
  if (player1.score >= SCORE_MAX) {
    unsigned long loopStart = millis();
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds, NUM_LEDS / 2, (millis() - loopStart));
      FastLED.show();
    }
  } else if (player2.score >= SCORE_MAX) {
    unsigned long loopStart = millis();
    while (millis() - loopStart < VICTORY_DELAY) {
      fill_rainbow(leds + NUM_LEDS / 2, NUM_LEDS / 2, (millis() - loopStart));
      FastLED.show();
    }
  } else {
    ball.x = 1;
    ball.dx = 1;
    if(rand() %2 == 1) {
      ball.x = NUM_LEDS - 1;
      ball.dx = -1;
    }
  }


  player1.score = 0;
  player2.score = 0;

  FastLED.clear();
  fill_solid(leds + NUM_LEDS / 2 - 2, 4, CRGB::White);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 4, 8, CRGB::White);
  FastLED.show();
  delay(600);
  fill_solid(leds + NUM_LEDS / 2 - 6, 12, CRGB::White);
  FastLED.show();
  delay(600);
  
  gameSetup = true;
}

void player1Win() {
  player1.score++;
  fill_rainbow_circular(leds + NUM_LEDS / 3, NUM_LEDS / 6, 100, false);
  FastLED.show();
  resetSet();
  delay(1000);
}

void player2Win() {
  player2.score++;
  fill_rainbow_circular(leds + NUM_LEDS / 2, NUM_LEDS / 6, 100, true);
  FastLED.show();
  resetSet();
  delay(1000);
}

// ---------------------- REFLEXE GAME-----------------------------

unsigned long start = 0;

void reflexeLoop() {
  if (!gameSetup) {
    reflexSetup();
  }

  Serial.println(millis() - start);
  
  if (digitalRead(PLAYER1_PIN) == LOW) {
    unsigned long delaiReaction = millis() - start;
    FastLED.clear();

    fill_solid(leds, delaiReaction / 3, CRGB::Red);
    FastLED.show();

    delay(5000);
    gameSetup = false;
    return;
  }
  
  if (digitalRead(PLAYER2_PIN) == LOW) {
    unsigned long delaiReaction = millis() - start;
    FastLED.clear();

    fill_solid(leds + NUM_LEDS - delaiReaction / 3, delaiReaction / 3, CRGB::Red);
    FastLED.show();

    delay(5000);
    gameSetup = false;
    return;
  }
}

void reflexSetup() {
  FastLED.clear();
  fill_solid(leds + NUM_LEDS / 2 - 2, 4, CRGB::White);
  FastLED.show();
  delay(rand() %200 + 400);
  fill_solid(leds + NUM_LEDS / 2 - 4, 8, CRGB::White);
  FastLED.show();
  delay(rand() %200 + 400);
  fill_solid(leds + NUM_LEDS / 2 - 6, 12, CRGB::White);
  FastLED.show();
  delay(rand() %200 + 400);
  fill_solid(leds + NUM_LEDS / 2 - 8, 16, CRGB::White);
  FastLED.show();
  
  delay(rand() %2000 + 500);

  FastLED.clear();
  FastLED.show();

  start = millis();
  gameSetup = true;
}

// ---------------------- Push GAME-----------------------------

void pushLoop() {
  if (!gameSetup) {
    pushSetup();
  }

  FastLED.clear();

  
  // Gestion des interactions avec les joueurs
  if (digitalRead(PLAYER1_PIN) == LOW && !player1.press) {
    player1.press = true;
    ball.x++;
  }

  if (digitalRead(PLAYER2_PIN) == LOW && !player2.press) {
    player2.press = true;
    ball.x--;
  }

  // eviter le hack de rester appuyé sur le bouton
  if (digitalRead(PLAYER1_PIN) == HIGH) {
    player1.press = false;
  }

  if (digitalRead(PLAYER2_PIN) == HIGH) {
    player2.press = false;
  }

  FastLED.clear();
  leds[(int)ball.x] = CRGB::White;
  FastLED.show();
}

void pushSetup() {
  ball.x = NUM_LEDS / 2;
  gameSetup = true;
}
