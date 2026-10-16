#include <Adafruit_LiquidCrystal.h>

// El índice 0 equivale a la dirección base 32 (0x20) en esta librería
Adafruit_LiquidCrystal lcd(0);

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Hola Mundo!");
}

void loop() {
}
