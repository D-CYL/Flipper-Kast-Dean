#include <Servo.h>
#include <Wire.h>              //communicatie I2C
#include <Adafruit_GFX.h>      //library voor de tekst
#include <Adafruit_SSD1306.h>  //library voor de oled scherm

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


// Declaratie voor een SSD1306-scherm aangesloten via I2C (SDA- en SCL-pinnen)
#define OLED_RESET -1  // Resetpin (of -1 als de Arduino-resetpin gedeeld wordt)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Servo leftServo;
Servo rightServo;

// Speler info
String name = "Dean";
String machine = "Flipperkast_Dean";

// Ultrasoon variabelen
const int trigPin = 7;
const int echoPin = 8;
float duration, distance;

// Variabelen voor de flippers
const int flipperButtonLeft = 2;
const int flipperButtonRight = 3;
const int flipperUp = 100;  // Hoek waarin de flipper actief staat
const int servoMotorLeft = 4;
const int servoMotorRight = 5;

// IR-sensor detecteert wanneer de bal passeert
const int irSense = 13;

// Spelvariabelen
long points = 0;
int lives = 3;


// Houdt bij of het spel al gestart is
bool gameStarted = false;

// Tijdstip waarop voor het laatst punten zijn verdiend
unsigned long lastPointTime = 0;

// Tijdstip waarop de bal voor het laatst is gedetecteerd
unsigned long lastDetectionTime = 0;

// Cooldown voordat opnieuw punten verdiend kunnen worden
const unsigned long pointCooldown = 3000;

// Tijd zonder baldetectie voordat een leven verloren gaat
const unsigned long lifeTimeout = 5000;

void setup() {
  Serial.begin(9600);
  pinMode(irSense, INPUT);

  // Initialiseer met het I2C-adres 0x3C (gebruikelijk voor 128x64-schermen)
  // Werkt dit niet, probeer dan adres 0x3D
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 initialisatie mislukt"));
    for (;;)
      ;  // Niet verder gaan, blijf oneindig herhalen
  }

  // Scherm leegmaken
  display.clearDisplay();

  // Tekst op scherm weergeven
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Instellen pins ultrasoon
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Instellen van de pinnen voor de flipperknoppen
  pinMode(flipperButtonLeft, INPUT_PULLUP);   // Knop indrukken = LOW
  pinMode(flipperButtonRight, INPUT_PULLUP);  // Knop indrukken = LOW

  // // Servo's aansluiten
  leftServo.attach(servoMotorLeft);    // servo aansluiten op pin 4
  rightServo.attach(servoMotorRight);  // servo aansluiten op pin 5

  // Beginpositie van de Servo's instellen
  leftServo.write(0);
  rightServo.write(flipperUp);
}

void loop() {
  // Alleen meten zolang het spel nog niet gestart is
  if (!gameStarted) {
  ultraSonic();

  // Alleen starten als er iets binnen 10 cm van de sensor zit
  if (distance <= 10.0 && distance > 0.0) {

    for (int countdown = 3; countdown > 0; countdown--) {
      display.clearDisplay();
      display.setCursor(0, 0);

      display.println("Start");
      display.print("in ");
      display.println(countdown);

      display.display();

      delay(1000);
    }

    gameStarted = true;
    lastDetectionTime = millis();
  }
}

  flipperButtons();

  int detectSensor = digitalRead(irSense);

  // Scherm leegmaken
  display.clearDisplay();

  // Tekst op scherm weergeven
  display.setCursor(0, 0);

  if (gameStarted == true) {  //Tekst bij begin van het spel
    display.println("Spel gestart!");
    display.println();
    remainingLives();
    display.println();
  } else {
    display.println("Zet je hand voor");  // Start tekst op het oled scherm
    display.println("de blauwe");
    display.println("speaker om het");
    display.println("spel te starten");
  }

  display.println();
  display.println(points);

  display.display();

  if (gameStarted) {

    if (detectSensor == LOW) {

      // Opslaan wanneer de bal voor het laatst gezien is
      lastDetectionTime = millis();

      if (millis() - lastPointTime >= pointCooldown) {   // Reset de tijd van millis() naar 0 zodat de cooldown kan worden getelt
        points += 100;
        lastPointTime = millis();
      }
    }

    // 5 seconden geen bal gezien = leven kwijt
    if (millis() - lastDetectionTime >= lifeTimeout) {

      lives--;

      lastDetectionTime = millis();

      if (lives <= 0) {
        endGame();
      }
    }
  }
}

//Ultrasoon sensor logica
void ultraSonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Verstuur een ultrasone puls van 10 microseconden
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  
  distance = (duration * 0.0343) / 2; 
  // Bereken de afstand in centimeters
  // 0,0343 cm/µs is de snelheid van geluid
  // Delen door 2 omdat het geluid heen en terug reist 

  // Serial.print("Distance: ");
  // Serial.println(distance);
}

//Flipper logica
void flipperButtons() {
  // Status van de linker flipperknop uitlezen
  int flipperStatusLeft = digitalRead(flipperButtonLeft);

  if (flipperStatusLeft == LOW) {
    leftServo.write(flipperUp);
  } else {
    leftServo.write(0);
  }

  // Status van de rechter flipperknop uitlezen
  int flipperStatusRight = digitalRead(flipperButtonRight);

  if (flipperStatusRight == LOW) {
    rightServo.write(0);
  } else {
    rightServo.write(flipperUp);
  }
}

// Verstuur eindscore via de seriële monitor
// Formaat: naam,score,flipperkast
void endGame() {
  display.clearDisplay();
  display.setCursor(0, 0);

  display.println("Geen levens meer!");
  display.println();
  display.println("Einde spel!");

  display.display();

  // Speler data verzenden met via serial
  Serial.print(name);
  Serial.print(",");
  Serial.print(String(points));
  Serial.print(",");
  Serial.println(machine);

  delay(10000);  // 10 seconden wachttijd voordat het spel reset

  // Spel reset naar de originele waarden
  lives = 3;
  points = 0;
  gameStarted = false;
}

// Tekst voor overige levens
void remainingLives() {
  switch (lives) {
    case 3:
      display.print("3 levens over!");
      break;
    case 2:
      display.print("2 levens over!");
      break;
    case 1:
      display.print("1 leven over!");
      break;
    default:  //Voor de mogelijkheid om meerdere levens te kunnen winnen (dan kan je bijvoorbeeld 4 levens op het scherm zien)
      display.print(lives);
      display.print(" levens over!");
      break;
  }
}