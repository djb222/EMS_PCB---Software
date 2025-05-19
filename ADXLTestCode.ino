#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD setup (adjust address if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Analog pins for ADXL335
const int xPin = A0; // Connect Xout to A0
const int yPin = A1; // Connect Yout to A1
const int zPin = A2; // Connect Zout to A2

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
}

void loop() {
  // Read analog values (0-1023)
  int xRaw = analogRead(xPin);
  int yRaw = analogRead(yPin);
  int zRaw = analogRead(zPin);

  // Convert to voltage (for 5V Arduino)
  float xVolt = xRaw * (5.0 / 1023.0);
  float yVolt = yRaw * (5.0 / 1023.0);
  float zVolt = zRaw * (5.0 / 1023.0);

  // Display on LCD
  lcd.setCursor(0, 0);
  lcd.print("X:");
  lcd.print(xVolt, 2);
  lcd.print(" Y:");
  lcd.print(yVolt, 2);

  lcd.setCursor(0, 1);
  lcd.print("Z:");
  lcd.print(zVolt, 2);
  lcd.print("V        "); // Clear remainder

  // Optional: Print to Serial Monitor
  Serial.print("X: "); Serial.print(xVolt, 2); Serial.print(" V, ");
  Serial.print("Y: "); Serial.print(yVolt, 2); Serial.print(" V, ");
  Serial.print("Z: "); Serial.print(zVolt, 2); Serial.println(" V");

  delay(500); // Update twice per second
}
