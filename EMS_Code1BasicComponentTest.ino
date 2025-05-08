#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin assignments
const int stationaryLED = 5;
const int walkingLED    = 6;
const int runningLED    = 7;
const int button1Pin    = 8; // Calibration
const int button2Pin    = 9; // Routine switch/reset

// Function Prototypes
void setupLEDs();
void setupButtons();
void setupLCD();
void updateLEDs(int button1State, int button2State);
void showStartupMessage();

void setup() {
  setupLEDs();
  setupButtons();
  setupLCD();
  showStartupMessage();
}

void loop() {
  int button1State = digitalRead(button1Pin);
  int button2State = digitalRead(button2Pin);

  updateLEDs(button1State, button2State);

  // Add other modular functions here as you expand features
}

// --- Modular Functions ---

void setupLEDs() {
  pinMode(stationaryLED, OUTPUT);
  pinMode(walkingLED, OUTPUT);
  pinMode(runningLED, OUTPUT);
  digitalWrite(stationaryLED, LOW);
  digitalWrite(walkingLED, LOW);
  digitalWrite(runningLED, LOW);
}

void setupButtons() {
  pinMode(button1Pin, INPUT); // Use INPUT_PULLUP if you don't have a pull-down resistor
  pinMode(button2Pin, INPUT);
}

void setupLCD() {
  lcd.init();
  lcd.backlight();
}

void showStartupMessage() {
  lcd.setCursor(0, 0);
  lcd.print("Hello, World!");
  lcd.setCursor(0, 1);
  lcd.print("LCD test ready");
}

void updateLEDs(int button1State, int button2State) {
  digitalWrite(stationaryLED, button1State == HIGH ? HIGH : LOW);
  digitalWrite(walkingLED,    button2State == HIGH ? HIGH : LOW);
  digitalWrite(runningLED, LOW); // Not used yet
}
