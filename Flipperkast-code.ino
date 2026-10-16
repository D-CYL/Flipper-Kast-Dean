#include <Servo.h>
#include <Wire.h> //communicatie I2C
#include <Adafruit_GFX.h> //library voor de tekst
#include <Adafruit_SSD1306.h> //library voor de oled scherm

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


// Declaratie voor een SSD1306-scherm aangesloten via I2C (SDA- en SCL-pinnen)
#define OLED_RESET     -1 // Resetpin (of -1 als de Arduino-resetpin gedeeld wordt)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


Servo leftServo;
Servo rightServo;

int flipperLeft = 2;   // pinnummer voor de linker flipperknop
int flipperRight = 4;  // pinnummer voor de rechter flipperknop
int flipperUp = 100;   // aantal graden voor de flipper
int irSense = 13;
int points = 0;
int score = 0;
int previousLeftState = HIGH;
int previousRightState = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(irSense, INPUT);

  // Initialiseer met het I2C-adres 0x3C (gebruikelijk voor 128x64-schermen)
  // Werkt dit niet, probeer dan adres 0x3D
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 initialisatie mislukt"));
    for(;;); // Niet verder gaan, blijf oneindig herhalen
  }

  // Scherm leegmaken
  display.clearDisplay();

  // Tekst op scherm weergeven
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  // Instellen van de pinnen voor de flipperknoppen
  // pinMode(flipperLeft, INPUT_PULLUP);   // indrukken geeft LOW
  // pinMode(flipperRight, INPUT_PULLUP);  // indrukken geeft LOW

  // // Servo's aansluiten
  // leftServo.attach(9);   // servo aansluiten op pin 9
  // rightServo.attach(10); // servo aansluiten op pin 10
}

void loop() {
  int detectSensor = digitalRead(irSense);

  // Scherm leegmaken
  display.clearDisplay();

  // Tekst op scherm weergeven
  display.setCursor(0, 0);
  display.println("Hello, World!!!\n");
  display.println();
  display.println(points);

  display.display();

  // Status van de linker flipperknop uitlezen
  // int flipperStatusLeft = digitalRead(flipperLeft);

  // if (flipperStatusLeft == LOW) {
  //   leftServo.write(flipperUp);
  //   // if (previousLeftState == HIGH) {
  //   //   points++;
  //   // Serial.print("Punten: "); 
  //   // Serial.println(points);
  //   // }
  // } else {
  //   leftServo.write(0);
  // }

  // // Status van de rechter flipperknop uitlezen
  // int flipperStatusRight = digitalRead(flipperRight);

  // if (flipperStatusRight == LOW) {
  //   rightServo.write(flipperUp);
  //   // if (previousRightState == HIGH) {
  //   //   points++;
  //   // Serial.print("Punten: "); 
  //   // display.println(points);
  //   // }
  // } else {
  //   rightServo.write(0);
  // }



  if (detectSensor == LOW) {
  Serial.println("Iets in de weg");
    points++;
    display.println(points);
  } else {
  Serial.println("Niks in de weg");
  }
}