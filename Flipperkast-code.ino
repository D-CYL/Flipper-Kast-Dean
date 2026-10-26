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

//TODO: Ultrasonic sensor
const int trigPin = 7;
const int echoPin = 8;
float duration, distance;

int flipperButtonLeft = 2;   // pinnummer voor de linker flipperknop
int flipperButtonRight = 3;  // pinnummer voor de rechter flipperknop
int flipperUp = 100;         // aantal graden voor de flipper
int irSense = 13;

//oled scherm data
int points = 0;
int lives = 3;
bool gameStarted = false;

//Status binnen spel
unsigned long lastPointTime = 0;
const unsigned long pointCooldown = 3000; // 3 seconden

int previousLeftState = HIGH;
int previousRightState = HIGH;

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

  //Instellen pins ultrasoon
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Instellen van de pinnen voor de flipperknoppen
  pinMode(flipperButtonLeft, INPUT_PULLUP);   // indrukken geeft LOW
  pinMode(flipperButtonRight, INPUT_PULLUP);  // indrukken geeft LOW

  // // Servo's aansluiten
  leftServo.attach(9);   // servo aansluiten op pin 9
  rightServo.attach(10); // servo aansluiten op pin 10
}

void loop() {
  // Alleen meten zolang het spel nog niet gestart is
  if (!gameStarted) {
    ultraSonic();

    if (distance < 50.0) {
      gameStarted = true;
      Serial.println("Spel gestart!");
    }
  }

  int detectSensor = digitalRead(irSense);

  // Scherm leegmaken
  display.clearDisplay();

  // Tekst op scherm weergeven
  display.setCursor(0, 0);

  if (gameStarted == true) {
    display.println("Spel gestart!");
  } else {
    display.println("Zet je hand voor");
    display.println("de blauwe");
    display.println("speaker om het");
    display.println("spel te starten");
  }

  display.println();
  display.println(points);

  display.display();

  if (detectSensor == LOW && gameStarted == true) {
    if (millis() - lastPointTime >= pointCooldown) {
    
    // Serial.println("Iets in de weg");
    points += 100;
    lastPointTime = millis();
  }
  }
}

void ultraSonic() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = (duration * 0.0343) / 2;

  Serial.print("Distance: ");
  Serial.println(distance);
}

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