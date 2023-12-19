/*
   Code de fonctionnement pour le jeu PONG.
   Par les petits debrouillards.
*/

#include <FastLED.h>

// Definition des couleurs
const CRGB NOIR  = CRGB(0,   0,   0  );
const CRGB BLANC = CRGB(255, 255, 255);
const CRGB ROUGE = CRGB(255, 0,   0  );
const CRGB VERT  = CRGB(0,   255, 0  );
const CRGB BLEU  = CRGB(0,   0,   255);
const CRGB JAUNE = CRGB(255, 255, 0  );
const CRGB ROSE  = CRGB(255, 0,   255);
const CRGB CYAN  = CRGB(0,   255, 255);


/******************************
    PARAMETRES DE BRANCHEMENTS
 ******************************/

const int LED_PIN = 4;          // Numero de branchement de la bande de LEDs
const int BUTTON1_PIN = 2;      // Numero de branchement du premier bouton
const int BUTTON2_PIN = 3;      // Numero de branchement du deuxieme bouton


/***********************
    PARAMETRES GENERAUX
 ***********************/
const int   NUM_LEDS = 144;            // Nombre de LEDs sur la bande
const CRGB  PLAYER1_COLOR = ROUGE;       // Couleur player 1
const CRGB  PLAYER2_COLOR = BLEU;    // Couleur player 2
const int   BRIGHTNESS = 5;
const int   TIME_CHOIX_MODE = 5000;


/***********************
     PARAMETRES PONG
 ***********************/
const float BALL_SPEED = 0.5;   // Vitesse de la balle
const float ACCELERATION = 9;   // Accelleration de la balle a chaque tir, si ACCELERATION = 10 on augmente la vitesse de 10 pourcent a chaque tir
const int   HIT_ZONE = 15;       // Nombre de LED pendant lesquelles on peut renvoyer la balle
const int   MAX_SCORE = 5;

const CRGB  BALL_COLOR = BLANC;      // Couleur de la balle

/*******************************
     PARAMETRES TIR A LA CORDE
 *******************************/
const float INCREMENT = 0.05;   // De combien avance la corde lorsqu'on appuie sur le bouton
const float ROPE_SMOOTH = 1.0;  // Parametre servant a lisser la position de la corde

const float WAVE_LENGTH = 0.2;  // Taille des ondes
const float WAVE_SPEED =  1.0;  // Vitesse des ondes



/******************************
      VARIABLES GENERALES
 ******************************/
// Players
enum PLAYER
{
  PERSONNE,
  PLAYER1,
  PLAYER2
};

// Etats du jeu
enum GAMESTATE
{
  START,
  GAME,
  WIN,
};
// Etats du jeu
enum GAMEMODE
{
  PONG1,   // Mode pong acceleration
  PONG2,   // Mode pong puissance
  TAC    // Mode tir a la corde
};

unsigned long lastMillis = 0;
unsigned long beginTimer = 0;
unsigned int counter = 0;

PLAYER player = PLAYER1; // Prochain joueur a appuyer sur son bouton
PLAYER lastWinner = PERSONNE;

CRGB leds[NUM_LEDS];
GAMESTATE gameState = START;
GAMEMODE gameMode = PONG1;


/**********************************************
      FONCTIONS DE FONCTIONNEMENT GENERALES
 **********************************************/

// Fonction permettant de changer la couleur d'une LED relativement au player 1 ou 2
void ledColor(PLAYER player, int pos, CRGB color)
{
  if (player == PLAYER1)
  {
    leds[pos] = color;
  }
  else // player == PLAYER2
  {
    leds[NUM_LEDS - pos - 1] = color;
  }
}

// Fonction permettant de changer la couleur d'une LED
void ledColor(int pos, CRGB color)
{
  leds[pos] = color;
}

void showVictoryPlayer()
{
  int startDisplayWinner = millis();
  // Si un player a gagne, on affiche l'arc en ciel du cote du vainqueur
  if (lastWinner == PLAYER1)
  {
    while ((millis() - startDisplayWinner) < 3000) {
      fill_rainbow(leds, NUM_LEDS / 2, counter++, 7);
      FastLED.show();
    }
  }
  else if (lastWinner == PLAYER2)
  {
    while ((millis() - startDisplayWinner) < 3000) {
      fill_rainbow(leds + NUM_LEDS / 2, NUM_LEDS / 2, counter++, 7);
      FastLED.show();
    }
  }

  gameState = START;
  FastLED.clear();
}

/*********************************************
      VARIABLES DE FONCTIONNEMENT DU PONG
 *********************************************/
float ballSpeed = BALL_SPEED;  // Vitesse de la balle
float ballPosition = 1; // Position de la balle sur la bande de led (Si ballPosition = 0, la balle est devant le player 1. Si ballPosition = 1, la balle est devant le player 2)
int player1Score = 0;   // Score du player 1
int player2Score = 0;   // Score du player 2

/**********************************************
      FONCTIONS DE FONCTIONNEMENT DU PONG
 **********************************************/
// Fonction servant a afficher les scores
void showScore()
{
  // On commence par effacer toutes les couleurs de led
  FastLED.clear();

  // On allume le nombre de led correspondant au score du player 1
  for (int i = 0; i < player1Score; i++)
  {
    ledColor(PLAYER1, NUM_LEDS / 2 - (i + 1), PLAYER1_COLOR);
  }

  // On allume le nombre de led correspondant au score du player 2
  for (int i = 0; i < player2Score; i++)
  {
    ledColor(PLAYER2, NUM_LEDS / 2 - (i + 1), PLAYER2_COLOR);
  }

  // On envoie les nouvelles couleurs a la bande de led
  FastLED.show();

  // On fait clignotter trois fois
  if (lastWinner == PLAYER1)
  {
    for (int i = 0; i < 3; i++)
    {
      // On eteint la derniere LED pendant 0.5s
      delay(500);
      ledColor(PLAYER1, NUM_LEDS / 2 - player1Score, NOIR);
      FastLED.show();

      // On allume la derniere LED pendant 0.5s
      delay(500);
      ledColor(PLAYER1, NUM_LEDS / 2 - player1Score, PLAYER1_COLOR);
      FastLED.show();
    }
  }
  else // lastWinner == PLAYER2
  {
    for (int i = 0; i < 3; i++)
    {
      // On eteint la derniere LED pendant 0.5s
      delay(500);
      ledColor(PLAYER2, NUM_LEDS / 2 - player2Score, NOIR);
      FastLED.show();

      // On allume la derniere LED pendant 0.5s
      delay(500);
      ledColor(PLAYER2, NUM_LEDS / 2 - player2Score, PLAYER2_COLOR);
      FastLED.show();
    }
  }

  // Si la partie est terminee on va a l'affichage de fin
  if (player1Score == MAX_SCORE || player2Score == MAX_SCORE)
  {
    gameState = WIN;

    // On reinitialise les scores
    player1Score = 0;
    player2Score = 0;

    // On reinitialise la vitesse
    ballSpeed = BALL_SPEED;

    // On reinitialise les leds
    FastLED.clear();
  }
  // Sinon on reprend le jeu
  /*else
  {
    gameState = Game;
    ballSpeed = BALL_SPEED;
  }*/
}


/***************************************
      FONCTION PRINCIPALE DU PONG1
 ***************************************/
void pong1Loop()
{
  // Calcul du temps ecoule entre deux boucles
  unsigned long currentMillis = millis();

  // On calcule le numero de la LED allumee
  int ballLed = ballPosition * NUM_LEDS;

  // On s'assure que la position de la balle ne depasse pas la taille de la bande de LED
  ballLed = min(ballLed, NUM_LEDS - 1);

  // On regarde qui est en train de jouer
  if (player == PLAYER1)
  {
    // On regarde si le player a appuye sur son bouton et si le delai de debut de jeu est passe
    if (digitalRead(BUTTON1_PIN) == LOW && currentMillis - beginTimer > 500)
    {
      // Si la balle est hors de la zone de tir, l'autre player marque un point
      if (ballLed >= HIT_ZONE)
      {
        player2Score += 1;
        lastWinner = PLAYER2;
        ballPosition = 0;

        // On passe en mode affichage des scores
        showScore();

        // C'est a l'autre player de jouer
        player = PLAYER2;

        // Actualisation de la variable lastMillis
        lastMillis = millis();
      }
      else
      {
        // On accelere la balle
        ballSpeed *= 1.0 + ACCELERATION / 100;

        // C'est a l'autre player de jouer
        player = PLAYER2;
      }

      return;
    }

    // On fait avancer la balle
    ballPosition -= ballSpeed * (currentMillis - lastMillis) * 0.001f;

    // On regarde si la balle est sortie de la zone
    if (ballPosition < 0.0f)
    {
      // Si oui le player 2 marque un point
      player2Score += 1;
      lastWinner = PLAYER2;
      ballPosition = 0;

      // On passe en mode affichage des scores
      showScore();

      // C'est a l'autre player de jouer
      player = PLAYER2;

      // Actualisation de la variable lastMillis
      lastMillis = millis();
      return;
    }
  }
  else // player == PLAYER2
  {
    // On regarde si le player a appuye sur son bouton et si le delai de debut de jeu est passe
    if (digitalRead(BUTTON2_PIN) == LOW && currentMillis - beginTimer > 500)
    {
      // Si la balle est hors de la zone de tir, l'autre player marque un point
      if (ballLed < NUM_LEDS - HIT_ZONE)
      {
        player1Score += 1;
        lastWinner = PLAYER1;
        ballPosition = 1;

        // On passe en mode affichage des scores
        showScore();

        // C'est a l'autre player de jouer
        player = PLAYER1;

        // Actualisation de la variable lastMillis
        lastMillis = millis();
      }
      else
      {
        // On accelere la balle
        ballSpeed *= 1.1;

        // C'est a l'autre player de jouer
        player = PLAYER1;
      }

      return;
    }

    // On fait avancer la balle dans l'autre sens
    ballPosition += ballSpeed * (currentMillis - lastMillis) * 0.001f;

    // On regarde si la balle est sortie de la zone
    if (ballPosition >= 1)
    {
      // Si oui le player 1 marque un point
      player1Score += 1;
      lastWinner = PLAYER1;
      ballPosition = 1;

      // On passe en mode affichage des scores
      showScore();
      // C'est a l'autre player de jouer
      player = PLAYER1;

      // Actualisation de la variable lastMillis
      lastMillis = millis();
      return;
    }
  }

  ///// AFFICHAGE BANDE DE LEDs /////
  // Premierement on efface toutes les couleurs precedentes
  FastLED.clear();

  // Ensuite on allume faiblement les LEDs correspondant a la zone de chaque cote
  for (int i = 0; i < HIT_ZONE; i++)
  {
    // On allume de chaque cote
    ledColor(PLAYER1, i, PLAYER1_COLOR / 10);  // On divise la couleur par 10 pour la rendre 10 fois moins puissante
    ledColor(PLAYER2, i, PLAYER2_COLOR / 10);
  }

  // Ensuite on allume faiblement les LEDs correspondant aux scores
  // Pour le player 1
  for (int i = 0; i < player1Score; i++)
  {
    ledColor(PLAYER1, NUM_LEDS / 2 - (i + 1), PLAYER1_COLOR / 15);
  }
  // Pour le player 2
  for (int i = 0; i < player2Score; i++)
  {
    ledColor(PLAYER2, NUM_LEDS / 2 - (i + 1), PLAYER2_COLOR / 15);
  }

  // Ensuite on actualise la position de la balle
  // On donne la couleur de la led en fonction de si la balle est dans la zone d'un player ou non

  // Si la balle est dans le camp d'un des player, elle est rouge.
  if (ballLed < HIT_ZONE || ballLed >= NUM_LEDS - HIT_ZONE)
  {
    ledColor(ballLed, ROUGE);
  }
  // Si elle en est proche, elle est jaune
  else if (ballLed < 2 * HIT_ZONE || ballLed >= NUM_LEDS - 2 * HIT_ZONE)
  {
    ledColor(ballLed, JAUNE);
  }
  // Sinon la balle a sa couleur par defaut
  else
  {
    ledColor(ballLed, BALL_COLOR);
  }

  // On envoie la couleur des leds a la bande de leds
  FastLED.show();

  // On actualise la variable lastMillis pour la boucle suivante
  lastMillis = currentMillis;
}

/***************************************
      FONCTION PRINCIPALE DU PONG2
 ***************************************/
void pong2Loop()
{
  // Calcul du temps ecoule entre deux boucles
  unsigned long currentMillis = millis();

  // On calcule le numero de la LED allumee
  int ballLed = ballPosition * NUM_LEDS;

  // On s'assure que la position de la balle ne depasse pas la taille de la bande de LED
  ballLed = min(ballLed, NUM_LEDS - 1);

  // On regarde qui est en train de jouer
  if (player == PLAYER1)
  {
    // On regarde si le player a appuye sur son bouton et si le delai de debut de jeu est passe
    if (digitalRead(BUTTON1_PIN) == LOW && currentMillis - beginTimer > 500)
    {
      // Si la balle est hors de la zone de tir, l'autre player marque un point //------------------------------------------------TODO
      if (ballLed >= HIT_ZONE)
      {
        player2Score += 1;
        lastWinner = PLAYER2;
        ballPosition = 0;

        // On passe en mode affichage des scores
        showScore();

        // C'est a l'autre player de jouer
        player = PLAYER2;

        // Actualisation de la variable lastMillis
        lastMillis = millis();
      }
      else
      {
        // On accelere la balle
        ballSpeed *= 1.0 + ACCELERATION / 100;

        // C'est a l'autre player de jouer
        player = PLAYER2;
      }

      return;
    }

    // On fait avancer la balle
    ballPosition -= ballSpeed * (currentMillis - lastMillis) * 0.001f;

    // On regarde si la balle est sortie de la zone
    if (ballPosition < 0.0f)
    {
      // Si oui le player 2 marque un point
      player2Score += 1;
      lastWinner = PLAYER2;
      ballPosition = 0;

      // On passe en mode affichage des scores
      showScore();

      // C'est a l'autre player de jouer
      player = PLAYER2;

      // Actualisation de la variable lastMillis
      lastMillis = millis();
      return;
    }
  }
  else // player == PLAYER2
  {
    // On regarde si le player a appuye sur son bouton et si le delai de debut de jeu est passe
    if (digitalRead(BUTTON2_PIN) == LOW && currentMillis - beginTimer > 500)
    {
      // Si la balle est hors de la zone de tir, l'autre player marque un point
      if (ballLed < NUM_LEDS - HIT_ZONE)
      {
        player1Score += 1;
        lastWinner = PLAYER1;
        ballPosition = 1;

        // On passe en mode affichage des scores
        showScore();

        // C'est a l'autre player de jouer
        player = PLAYER1;

        // Actualisation de la variable lastMillis
        lastMillis = millis();
      }
      else
      {
        // On accelere la balle
        ballSpeed *= 1.1;

        // C'est a l'autre player de jouer
        player = PLAYER1;
      }

      return;
    }

    // On fait avancer la balle dans l'autre sens
    ballPosition += ballSpeed * (currentMillis - lastMillis) * 0.001f;

    // On regarde si la balle est sortie de la zone
    if (ballPosition >= 1)
    {
      // Si oui le player 1 marque un point
      player1Score += 1;
      lastWinner = PLAYER1;
      ballPosition = 1;

      // On passe en mode affichage des scores
      showScore();
      // C'est a l'autre player de jouer
      player = PLAYER1;

      // Actualisation de la variable lastMillis
      lastMillis = millis();
      return;
    }
  }

  ///// AFFICHAGE BANDE DE LEDs /////
  // Premierement on efface toutes les couleurs precedentes
  FastLED.clear();

  // Ensuite on allume faiblement les LEDs correspondant a la zone de chaque cote
  for (int i = 0; i < HIT_ZONE; i++)
  {
    // On allume de chaque cote
    ledColor(PLAYER1, i, PLAYER1_COLOR / 10);  // On divise la couleur par 10 pour la rendre 10 fois moins puissante
    ledColor(PLAYER2, i, PLAYER2_COLOR / 10);
  }

  // Ensuite on allume faiblement les LEDs correspondant aux scores
  // Pour le player 1
  for (int i = 0; i < player1Score; i++)
  {
    ledColor(PLAYER1, NUM_LEDS / 2 - (i + 1), PLAYER1_COLOR / 15);
  }
  // Pour le player 2
  for (int i = 0; i < player2Score; i++)
  {
    ledColor(PLAYER2, NUM_LEDS / 2 - (i + 1), PLAYER2_COLOR / 15);
  }

  // Ensuite on actualise la position de la balle
  // On donne la couleur de la led en fonction de si la balle est dans la zone d'un player ou non

  // Si la balle est dans le camp d'un des player, elle est rouge.
  if (ballLed < HIT_ZONE || ballLed >= NUM_LEDS - HIT_ZONE)
  {
    ledColor(ballLed, ROUGE);
  }
  // Si elle en est proche, elle est jaune
  else if (ballLed < 2 * HIT_ZONE || ballLed >= NUM_LEDS - 2 * HIT_ZONE)
  {
    ledColor(ballLed, JAUNE);
  }
  // Sinon la balle a sa couleur par defaut
  else
  {
    ledColor(ballLed, BALL_COLOR);
  }

  // On envoie la couleur des leds a la bande de leds
  FastLED.show();

  // On actualise la variable lastMillis pour la boucle suivante
  lastMillis = currentMillis;
}


/*******************************************************
      VARIABLES DE FONCTIONNEMENT DU TIR A LA CORDE
 *******************************************************/
float ropePosition = 0.5;           // Position non lissee de la corde
float displayedRopePosition = 0.5;  // Position lissee de la corde
bool player1Pushed = false;
bool player2Pushed = false;

float player1Waves[10] = { 0 };
float player2Waves[10] = { 0 };

/***********************************************
      FONCTION PRINCIPALE DU TIR A LA CORDE
 ***********************************************/
void tacLoop()
{  
  // Calcul des temps
  unsigned long ms = millis();
  float dt = (ms - lastMillis) * 0.001f;
  lastMillis = ms;

  // Si moins d'une seconde s'est ecoule depuis le debut, on ne fait rien (pour eviter d'appuyer sans faire expres)
  if (ms - beginTimer < 1000)
    return;

  bool player1Push = !digitalRead(BUTTON1_PIN);
  bool player2Push = !digitalRead(BUTTON2_PIN);

  if (player1Push && !player1Pushed)
  {
    // On incremente la position de la corde
    ropePosition += INCREMENT;

    // On lance une nouvelle onde
    for (int i = 0; i < 10; i++)
    {
      if (player1Waves[i] == 0.0f)
      {
        //player1Waves[i] = WAVE_SPEED * dt;
        player1Waves[i] = 0.01f;
        break;
      }
    }
  }
  if (player2Push && !player2Pushed)
  {
    // On incremente la position de la corde
    ropePosition -= INCREMENT;

    // On lance une nouvelle onde
    for (int i = 0; i < 10; i++)
    {
      if (player2Waves[i] == 0.0f)
      {
        player2Waves[i] = 0.99f;
        break;
      }
    }
  }

  // Memorisation des etats des boutons
  player1Pushed = player1Push;
  player2Pushed = player2Push;




  // On calcule la position lissee de la corde (displayedRopePosition correspond a la variable ropePosition avec un lissage supplementaire)
  displayedRopePosition += (ropePosition - displayedRopePosition) * dt / ROPE_SMOOTH;

  // On regarde si player 1 a gagne
  if (displayedRopePosition >= 1.0f)
  {
    lastWinner = PLAYER1;
    gameState = WIN;
    beginTimer = millis();

    ropePosition = 0.5f;
    displayedRopePosition = 0.5f;

    FastLED.clear();
    return;
  }
  // On regarde si player 2 a gagne
  else if (displayedRopePosition <= 0.0f)
  {
    lastWinner = PLAYER2;
    gameState = WIN;
    beginTimer = millis();

    ropePosition = 0.5f;
    displayedRopePosition = 0.5f;

    FastLED.clear();
    return;
  }

  // On propage les ondes
  float dx = WAVE_SPEED * dt;

  for (int wave = 0; wave < 10; wave++)
  {
    // Ondes player 1
    if (player1Waves[wave] != 0.0f)
    {
      player1Waves[wave] += dx;

      // Si toute l'onde depasse le point de la corde, on l'arrete
      if (player1Waves[wave] - WAVE_LENGTH > displayedRopePosition)
      {
        player1Waves[wave] = 0.0f;
      }
    }

    // Ondes player 2
    if (player2Waves[wave] != 0.0f)
    {
      player2Waves[wave] -= dx;

      // Si toute l'onde depasse le point de la corde, on l'arrete
      if (player2Waves[wave] + WAVE_LENGTH < displayedRopePosition)
      {
        player2Waves[wave] = 0.0f;
      }
    }
  }

  // On actualise la bande de leds
  float ledLuminosity;
  float ledPosition;

  FastLED.clear();

  // On commence par calculer le numero de la led correspondant a la position de la corde
  int ropeLedPosition = NUM_LEDS * displayedRopePosition;

  // On dessine la zone du player 1
  // Pour cela on calcule la luminosite led par led
  for (int led = 0; led < ropeLedPosition; led++)
  {
    // La luminosite minimale d'une led est 10% de la luminosite max
    ledLuminosity = 0.1f;
    ledPosition = float(led) / float(NUM_LEDS);

    // Pour chaque onde, on ajoute la luminosite a la led
    for (int wave = 0; wave < 10; wave++)
    {
      // Si l'onde n'est pas active ou si l'onde est avant la led, on n'ajoute aucune luminosite
      if (player1Waves[wave] == 0.0f || player1Waves[wave] < ledPosition)
        continue;

      // On ajoute de la luminosite de facon decroissante. plus l'onde est loin, moins on ajoute de luminosite.
      ledLuminosity += max(0.0f, 0.9f - (player1Waves[wave] - ledPosition) / WAVE_LENGTH);
    }
    // La valeur maximale de luminosite est 1.0
    if (ledLuminosity > 2.0f)
      ledLuminosity = 2.0f;

    // On actualise la valeur de luminosite de la led
    ledColor(led, PLAYER1_COLOR / int(2.0f / ledLuminosity) / 5);
  }

  // On dessine la zone du player 2
  for (int led = ropeLedPosition + 1; led < NUM_LEDS; led++)
  {
    // La luminosite minimale d'une led est 10% de la luminosite max
    ledLuminosity = 0.1f;
    ledPosition = float(led) / float(NUM_LEDS);

    // Pour chaque onde, on ajoute la luminosite a la led
    for (int wave = 0; wave < 10; wave++)
    {
      // Si l'onde n'est pas active ou si l'onde est avant la led, on n'ajoute aucune luminosite
      if (player2Waves[wave] == 0.0f || ledPosition > player2Waves[wave])
        continue;

      // On ajoute de la luminosite de facon decroissante. plus l'onde est loin, moins on ajoute de luminosite.
      ledLuminosity += max(0.0f, 0.9f - (player2Waves[wave] - ledPosition) / WAVE_LENGTH);
    }
    // La valeur maximale de luminosite est 1.0
    if (ledLuminosity > 2.0f)
      ledLuminosity = 2.0f;

    // On actualise la valeur de luminosite de la led
    ledColor(led, PLAYER2_COLOR / int(1.0f / ledLuminosity) / 5);
  }

  // On actualise la led correspondant a la position de la corde
  ledColor(ropeLedPosition, BLANC);
  FastLED.show();
}

void displayCurrentMode()
{
  switch (gameMode)
  {
    case PONG1:
      displayMiniPong1();
      break;
    case PONG2:
      displayMiniPong2();
      break;
    case TAC:
      displayMiniTac();
      break;
  }
}

void displayMiniRainbow()
{
  fill_rainbow(leds + 30, NUM_LEDS - 60, counter++, 1);
  FastLED.show();
}

void displayMiniPong1()
{
  unsigned long currentMillis = millis();
  // On commence par effacer toutes les couleurs de led
  FastLED.clear();

  for (int i = 0; i < 10; i++)
  {
    ledColor(PLAYER1, 30+i, PLAYER1_COLOR);
    ledColor(PLAYER2, 30+i, PLAYER2_COLOR);
  }

  int lenght_mini_board = NUM_LEDS - 40 * 2;
  
  ballPosition += ballSpeed * (currentMillis - lastMillis) * 0.001f;
  int ball_led = ballPosition;
  
  ball_led = ball_led % lenght_mini_board;


  ledColor(ballPosition + 39, BALL_COLOR);


  // On envoie les nouvelles couleurs a la bande de led
  FastLED.show();
}

void displayMiniPong2()
{
  // On commence par effacer toutes les couleurs de led
  FastLED.clear();
  // On envoie les nouvelles couleurs a la bande de led
  FastLED.show();
}

void displayMiniTac()
{
  // On commence par effacer toutes les couleurs de led
  FastLED.clear();
  // On envoie les nouvelles couleurs a la bande de led
  FastLED.show();
}

/****************************************************************
   Cette fonction s'execute une fois lorsque la carte s'allume.
 ****************************************************************/
void setup() {
  Serial.begin(9600);

  // Initialisation des LEDs
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  // Luminosité des leds
  FastLED.setBrightness(BRIGHTNESS);

  // Initialisations des boutons
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);

  // COULEUR DES LEDS EN DEBUT DE PARTIE
  FastLED.clear();
  // Couleurs player 1
  ledColor(PLAYER1, 0, PLAYER1_COLOR);
  ledColor(PLAYER1, 1, PLAYER1_COLOR);
  ledColor(PLAYER1, 2, PLAYER1_COLOR);
  ledColor(PLAYER1, 3, PLAYER1_COLOR);
  ledColor(PLAYER1, 4, PLAYER1_COLOR);

  // Couleurs player 2
  ledColor(PLAYER2, 0, PLAYER2_COLOR);
  ledColor(PLAYER2, 1, PLAYER2_COLOR);
  ledColor(PLAYER2, 2, PLAYER2_COLOR);
  ledColor(PLAYER2, 3, PLAYER2_COLOR);
  ledColor(PLAYER2, 4, PLAYER2_COLOR);

  // On envoie les changements a la bande de leds
  FastLED.show();

  // Initialisation du temps d'allumage
  beginTimer = millis();
}

/*************************************************************
   Cette fonction s'execute en continue tout au long du jeu.
 *************************************************************/
void loop() {
  switch (gameState)
  {
    case WIN:
      showVictoryPlayer();

    case START:
      displayCurrentMode();

      // On regarde si un temps minimim s'est ecoule et si un des boutons est appuye
      if (millis() - beginTimer > 100 && (digitalRead(BUTTON1_PIN) == LOW || digitalRead(BUTTON2_PIN) == LOW))
      {
        unsigned long pushBegin = millis();

        unsigned int nbLed = 0;

        while ((digitalRead(BUTTON1_PIN) == LOW || digitalRead(BUTTON2_PIN) == LOW) && (millis() - pushBegin) < TIME_CHOIX_MODE)
        {
          nbLed = (millis() - pushBegin) / 500 + 1;
          fill_rainbow(leds, nbLed, counter++, 7);
          //ledColor(PLAYER1, nbLed, VERT);
          FastLED.show();
        }

        if ((millis() - pushBegin) >= TIME_CHOIX_MODE)
        {
          Serial.println("go to game");
          gameState = GAME;
        }
        else if ((millis() - pushBegin) > 200)
        {
          Serial.println("FastLED.clear");
          FastLED.clear();
          gameMode = TAC;
          //gameMode; //------------------------------------------------TODO change next enum
        }

        // Initialisation de la varable lastMillis
        lastMillis = millis();

        // Initialisation du temps de debut de jeu
        beginTimer = millis();
      }

      break;

    case GAME:
      switch (gameMode)
      {
        case PONG1:
          pong1Loop();
          break;
        case PONG2:
          pong2Loop();
          break;
        case TAC:
          tacLoop();
          break;
      }
      break;
  }
}
