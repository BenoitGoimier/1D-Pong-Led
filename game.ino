// Projet Arduino du jeu pong en 1d avec un bandeau de led de 144 led sur le pin 4 et deux bouton sur le pin 2 et 3

// import de la library FastLED
#include <FastLED.h>

// definition des parametres de la bande de led
#define NUM_LEDS 144
#define DATA_PIN 4
#define BTN1_PIN 2
#define BTN2_PIN 3
#define BRIGHTNESS 10

// definition des couleurs
#define COLOR_BLACK CRGB(0, 0, 0)
#define COLOR_RED CRGB(255, 0, 0)
#define COLOR_GREEN CRGB(0, 255, 0)
#define COLOR_BLUE CRGB(0, 0, 255)
#define COLOR_YELLOW CRGB(255, 255, 0)
#define COLOR_PURPLE CRGB(255, 0, 255)
#define COLOR_CYAN CRGB(0, 255, 255)
#define COLOR_WHITE CRGB(255, 255, 255)

// definition des settings
#define PONG_SCORE_MAX 5
#define PLAYER_LED_SIZE 10
#define PLAYER_MISS_BALL_TIME 500
#define BALL_SPEED 1

// definition des variables
CRGB leds[NUM_LEDS];
Display display = ANIMATION;
GameMode gameMode = PONG;
Animation animation = RAINBOW;
int scorePlayer1 = 0;
int scorePlayer2 = 0;
float ballPosition = NUM_LEDS / 2 / NUM_LEDS;
int ballSpeed = 1;
int hitBallTime;
int player1MissBallTime = 0;
int player2MissBallTime = 0;

// definition des enumerations
enum Display
{
  START,
  GAME,
  ANIMATION,
  END
};

enum GameMode
{
  PONG,
  TIR_A_LA_CORDE,
  PONG_2
};

enum Animation
{
  RAINBOW,
  FIRE,
  BOUNCE,
  RANDOM,
  SNAKE
}

enum Player
{
  PLAYER_1,
  PLAYER_2
};

// definition des fonctions
void setup()
{
  // delai setup
  delay(1000);

  // initialisation du random
  randomSeed(analogRead(0));

  // initialisation du port serie
  Serial.begin(9600);

  // initialisation des parametres de la bande de led
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  // initialisation des parametres des boutons
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
}

void loop()
{
  // switch du display
  switch (display)
  {
  case START:
    start();
    break;
  case GAME:
    game();
    break;
  case ANIMATION:
    animation();
    break;
  case END:
    end();
    break;
  }
}

// fonction start
void start()
{
  // switch du gameMode pour préparer le jeu
  switch (gameMode)
  {
  case PONG:
    pongStart();
    break;
  case TIR_A_LA_CORDE:
    tir_a_la_cordeStart();
    break;
  case PONG_2:
    pongStart();
    break;
  }
}

void pongStart()
{
  // réinitialisation des scores
  scorePlayer1 = 0;
  scorePlayer2 = 0;

  FastLED.clear();

  displayPlayer();

  attachInterrupt(digitalPinToInterrupt(BTN1_PIN), player1HitPong, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN2_PIN), player2HitPong, FALLING);
}

void displayPlayer()
{
  // affiche les 2 joueurs aux deux extremités
  for (int i = 0; i < PLAYER_LED_SIZE; i++)
  {
    leds[i] = COLOR_RED;
    leds[NUM_LEDS - i] = COLOR_BLUE;
  }
  FastLED.show();
}

void tir_a_la_cordeStart()
{
  // réinitialisation des scores
  scorePlayer1 = 0;
  scorePlayer2 = 0;

  FastLED.clear();

  attachInterrupt(digitalPinToInterrupt(BTN1_PIN), player1Tac, FALLING);
  attachInterrupt(digitalPinToInterrupt(BTN2_PIN), player2Tac, FALLING);
}

void displayScore()
{
  for (int i = 0; i < scorePlayer1; i++)
  {
    leds[NUM_LEDS / 2 - i] = COLOR_RED;
  }
  for (int i = 0; i < scorePlayer2; i++)
  {
    leds[NUM_LEDS / 2 + i] = COLOR_BLUE;
  }
}

// fonction game
void game()
{
  // affichage du message de debut
  Serial.println("GAME");

  // switch du gameMode
  switch (gameMode)
  {
  case PONG:
    pong();
    break;
  case TIR_A_LA_CORDE:
    tir_a_la_corde();
    break;
  case PONG_2:
//    pong_2();
    break;
  }
}

/**************
 * PONG *
***************/

// fonction end
void end()
{
  // affichage du message de debut
  Serial.println("END");

  // affichage du gagnant avec la moitié du ruban de son coté en rainbow
  if (scorePlayer1 >= PONG_SCORE_MAX)
  {
    fill_rainbow(leds, NUM_LEDS / 2, 0, 255 / NUM_LEDS);
  }
  else
  {
    fill_rainbow(leds + NUM_LEDS / 2, NUM_LEDS / 2, 0, 255 / NUM_LEDS);
  }
}

// fonction pong
void pong()
{
  // affichage du message de debut
  Serial.println("PONG");

  // affichage des scores
  displayScore();

  // deplacement de la balle
  moveBall();

  // affichage de la balle
  displayBall();

  // check si la balle est sortie
  checkBall();

  // check fin de partie
  if (scorePlayer1 >= PONG_SCORE_MAX || scorePlayer2 >= PONG_SCORE_MAX)
  {
    display = END;
  }
}

void displayBall()
{
  // affichage de la balle
  leds[ballPosition] = COLOR_WHITE;
}

void moveBall()
{
  // deplacement de la balle vers le joueur qui doit la recevoir
  ballPosition += hitBallTime - millis() * ballSpeed / 1000;
}

void checkBall()
{
  // check si la balle est sortie
  if (ballPosition * NUM_LEDS < 0)
  {
    // si la balle est sortie a gauche
    scorePlayer2++;
  }
  else if (ballPosition * NUM_LEDS > NUM_LEDS)
  {
    // si la balle est sortie a droite
    scorePlayer1++;
  }
}

void player1HitPong()
{
  int time = millis();
  //check si le joueur peut frapper la balle
  if (time - player1MissBallTime < PLAYER_MISS_BALL_TIME)
  {
    return;
  }

  // check si la balle n'a pas encore atteint le joueur
  if (ballPosition * NUM_LEDS > PLAYER_LED_SIZE)
  {
    // désactive la frappe du joueur pendant 500ms
    player1MissBallTime = time;
    return;
  }

  // si la balle est sur le joueur, on regarde la position pour définir la puissance de la frappe
  int power = PLAYER_LED_SIZE - ballPosition + 1;
  ballSpeed = BALL_SPEED * power;
  hitBallTime = time;
}

void player2HitPong()
{
  int time = millis();
  //check si le joueur peut frapper la balle
  if (time - player2MissBallTime < PLAYER_MISS_BALL_TIME)
  {
    return;
  }

  // check si la balle n'a pas encore atteint le joueur
  if (ballPosition * NUM_LEDS < NUM_LEDS - PLAYER_LED_SIZE)
  {
    // désactive la frappe du joueur pendant 500ms
    player2MissBallTime = time;
    return;
  }

  // si la balle est sur le joueur, on regarde la position pour définir la puissance de la frappe
  int power = PLAYER_LED_SIZE - NUM_LEDS - ballPosition + 1;
  // on inverse la direction de la balle
  ballSpeed = BALL_SPEED * power * -1;
  hitBallTime = time;
}


/**************
 * Tir à la corde *
***************/

void tir_a_la_corde()
{
  // affichage du message de debut
  Serial.println("TIR_A_LA_CORDE");

  //affichage de la balle
  displayBall();

  checkBallTac();
}

void player1Tac()
{
  ballPosition += 0.5f;
}

void player2Tac()
{
  ballPosition -= 0.5f;
}

void checkBallTac()
{
  // check si la balle est sortie
  if (ballPosition * NUM_LEDS < 0)
  {
    // si la balle est sortie a gauche
    scorePlayer2++;
    display = END;
  }
  else if (ballPosition * NUM_LEDS > NUM_LEDS)
  {
    // si la balle est sortie a droite
    scorePlayer1++;
    display = END;
  }
}

/**************
 * Animation *
***************/

// fonction animation (veille)
void animation()
{
  switch (animation)
  {
  case RAINBOW:
    rainbow();
    break;
  case FIRE:
    fire();
    break;
  case BOUNCE:
    bounce();
    break;
  case RANDOM:
    random();
    break;
  case SNAKE:
    snake();
    break;
  }
}

void rainbow()
{
  Serial.println("RAINBOW");
  FastLED.clear();
  fill_rainbow(leds, NUM_LEDS, 0, 255 / NUM_LEDS);
}

void fire()
{
  Serial.println("FIRE");
  FastLED.clear();
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = HeatColor(random(255));
  }
}

void bounce()
{
  Serial.println("BOUNCE");
  FastLED.clear();
  leds[random(NUM_LEDS)] = CRGB::White;
}

void random()
{
  Serial.println("RANDOM");
  FastLED.clear();
  leds[random(NUM_LEDS)] = CRGB::White;
}

void snake()
{
  Serial.println("SNAKE");
  FastLED.clear();
  leds[random(NUM_LEDS)] = CRGB::White;
}
