#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin assignments
const int stationaryLED = 5;
const int walkingLED    = 6;
const int runningLED    = 7;
const int button1Pin    = 8;  // Mode/Select
const int button2Pin    = 9;  // Reset/Confirm

// System modes
enum Mode { NORMAL, SETUP, COUNTDOWN, RESULTS};
Mode currentMode = NORMAL;

// Distance tracking
const float STEP_LENGTH = 0.7; // Average meters per step
const int distanceOptions[3] = {500, 1000, 2000};
int selectedDistanceIndex = 0;
float remainingDistance = 0;
unsigned long countdownStartTime = 0;
unsigned long totalTime = 0;
float averagePace = 0; // minutes per kilometer

// Step simulation and pace tracking
enum PaceState { STATIONARY, WALKING, RUNNING };
PaceState currentPace = STATIONARY;
unsigned long lastStepTime = 0;
int stepCount = 0;
float simulatedAccel = 0;
const float stepThreshold = 0.7;
bool aboveThreshold = false;

// Timing and state control
unsigned long lastStateChange = 0;
const unsigned long minStateDuration = 5000;
const unsigned long maxStateDuration = 15000;
unsigned long stateDuration = 0;

// Button handling
const unsigned long LONG_PRESS_TIME = 1000;
unsigned long buttonPressStart = 0;
bool button1Active = false;
bool button2Active = false;

// Function Prototypes
void handleButtons(unsigned long currentTime);
void handleLongPress();
void handleShortPress();
void handleButton2();
void enterSetupMode();
void startCountdown();
void cancelCountdown();
void cycleDistanceOptions();
void generateSensorData(unsigned long currentTime);
void detectSteps(unsigned long currentTime);
void changeState();
void updateDisplay();
void displayNormalMode();
void displaySetupMode();
void displayCountdownMode();
void updateLEDs();
void resetSystem();

void setup() {
  pinMode(stationaryLED, OUTPUT);
  pinMode(walkingLED, OUTPUT);
  pinMode(runningLED, OUTPUT);
  pinMode(button1Pin, INPUT);
  pinMode(button2Pin, INPUT);

  lcd.init();
  lcd.backlight();
  randomSeed(analogRead(A0));
  changeState();
}

void loop() {
  unsigned long currentTime = millis();
  static unsigned long lastLCDUpdate = 0;
  static unsigned long lastSimTime = 0;

  handleButtons(currentTime);
  
  if (currentMode == NORMAL || currentMode == COUNTDOWN) {
    // Update pace state machine
    if (currentTime - lastStateChange > stateDuration) {
      changeState();
    }
    
    // Generate simulated sensor data at 50Hz
    if (currentTime - lastSimTime > 20) {
      lastSimTime = currentTime;
      generateSensorData(currentTime);
      detectSteps(currentTime);
    }
  }

  // Update display every 200ms
  if (currentTime - lastLCDUpdate > 200) {
    updateDisplay();
    updateLEDs();
    lastLCDUpdate = currentTime;
  }
}

// --- Button Handling ---
void handleButtons(unsigned long currentTime) {
  // Handle Button 1 (Mode/Select)
  if (digitalRead(button1Pin) == HIGH) {
    if (!button1Active) {
      buttonPressStart = currentTime;
      button1Active = true;
    }
    // Long press detection
    if (currentTime - buttonPressStart > LONG_PRESS_TIME) {
      handleLongPress();
      button1Active = false; // Prevent retriggering
    }
  } else {
    if (button1Active) {
      handleShortPress();
      button1Active = false;
    }
  }

  // Handle Button 2 (Reset/Confirm)
  if (digitalRead(button2Pin) == HIGH) {
    if (!button2Active) {
      handleButton2();
      button2Active = true;
    }
  } else {
    button2Active = false;
  }
}

void handleLongPress() {
  switch(currentMode) {
    case NORMAL:
      enterSetupMode();
      break;
    case SETUP:
      startCountdown();
      break;
    default:
      break;
  }
}

void handleShortPress() {
  if (currentMode == SETUP) {
    cycleDistanceOptions();
    updateDisplay(); // Immediate feedback
  }
}

void handleButton2() {
  switch(currentMode) {
    case RESULTS: //ADD RESET FROM RESULTS MODE
    case NORMAL:
      resetSystem();
      break;
    case COUNTDOWN:
      cancelCountdown();
      break;
    default:
      break;
  }
}

// --- Mode Transitions ---
void enterSetupMode() {
  currentMode = SETUP;
  selectedDistanceIndex = 0;
  lcd.clear();
}

void startCountdown() {
  currentMode = COUNTDOWN;
  remainingDistance = distanceOptions[selectedDistanceIndex];
  stepCount = 0;
  countdownStartTime = millis();  // <-- RECORD START TIME
  lcd.clear();
}

void cancelCountdown() {
  currentMode = NORMAL;
  remainingDistance = 0;
  lcd.clear();
}

void cycleDistanceOptions() {
  selectedDistanceIndex = (selectedDistanceIndex + 1) % 3;
}

// --- Core Logic ---
void generateSensorData(unsigned long currentTime) {
  float freq = 0.0;
  switch(currentPace) {
    case WALKING: freq = 1.5; break;
    case RUNNING: freq = 3.0; break;
    default: freq = 0.0;
  }
  
  simulatedAccel = (freq > 0) ? sin(2 * PI * freq * (currentTime / 1000.0)) : 0;
  simulatedAccel += random(-10, 10) / 100.0; // Add noise
}

void detectSteps(unsigned long currentTime) {
  if (simulatedAccel > stepThreshold && !aboveThreshold) {
    aboveThreshold = true;
    stepCount++;
    if (currentMode == COUNTDOWN) {
      remainingDistance = max(0.0, remainingDistance - STEP_LENGTH);
      
      // Check if goal reached
      if (remainingDistance <= 0) {
        totalTime = (currentTime - countdownStartTime) / 1000; // in seconds
        calculateAveragePace();
        currentMode = RESULTS;
        lcd.clear();
      }
    }
    lastStepTime = currentTime;
  }
  if (simulatedAccel < stepThreshold) {
    aboveThreshold = false;
  }
}

void calculateAveragePace() {
  float distanceKm = distanceOptions[selectedDistanceIndex] / 1000.0;
  float timeHours = totalTime / 3600.0;
  averagePace = (timeHours / distanceKm) * 60; // minutes per kilometer
}

void changeState() {
  int randNum = random(100);
  if (randNum < 10) currentPace = RUNNING;
  else if (randNum < 70) currentPace = WALKING;
  else currentPace = STATIONARY;

  stateDuration = random(minStateDuration, maxStateDuration);
  lastStateChange = millis();
}

// --- Display Updates ---
void updateDisplay() {
  lcd.clear();
  
  switch(currentMode) {
    case NORMAL:
      displayNormalMode();
      break;
    case SETUP:
      displaySetupMode();
      break;
    case COUNTDOWN:
      displayCountdownMode();
      break;
    case RESULTS:
      displayResultsMode();
      break;
    default:
      break;
  }
}

void displayNormalMode() {
  lcd.setCursor(0, 0);
  lcd.print("Steps: ");
  lcd.print(stepCount);
  
  lcd.setCursor(0, 1);
  lcd.print("Pace: ");
  switch(currentPace) {
    case STATIONARY: lcd.print("Stationary"); break;
    case WALKING:    lcd.print("Walking   "); break;
    case RUNNING:    lcd.print("Running   "); break;
  }
}

void displaySetupMode() {
  lcd.setCursor(0, 0);
  lcd.print("Set Distance Goal:");
  
  lcd.setCursor(0, 1);
  lcd.print(distanceOptions[selectedDistanceIndex]);
  lcd.print(" meters");
}

void displayCountdownMode() {
  lcd.setCursor(0, 0);
  lcd.print("Goal: ");
  lcd.print(distanceOptions[selectedDistanceIndex]);
  lcd.print("m");
  
  lcd.setCursor(0, 1);
  lcd.print("Left: ");
  lcd.print((int)remainingDistance);
  lcd.print("m");
}

void updateLEDs() {
  digitalWrite(stationaryLED, currentPace == STATIONARY ? HIGH : LOW);
  digitalWrite(walkingLED,    currentPace == WALKING    ? HIGH : LOW);
  digitalWrite(runningLED,    currentPace == RUNNING    ? HIGH : LOW);
}

void displayResultsMode() {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(totalTime / 60);
  lcd.print("m ");
  lcd.print(totalTime % 60);
  lcd.print("s");
  
  lcd.setCursor(0, 1);
  lcd.print("Pace: ");
  lcd.print(averagePace, 1);
  lcd.print(" min/km");
}


// --- System Control ---
void resetSystem() {
  stepCount = 0;
  remainingDistance = 0;
  currentMode = NORMAL;
  lcd.clear();
}
