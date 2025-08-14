#include <Arduino.h>
#include <Wire.h>

#define SDA_PIN 6
#define SCL_PIN 7
#define BH1750_ADDR 0x23

// BH1750 Befehle
#define BH1750_POWER_DOWN   0x00
#define BH1750_POWER_ON     0x01
#define BH1750_RESET        0x07  // nur gültig nach POWER_ON
#define BH1750_CONT_HIRES1  0x10  // 1 lx Auflösung, ~120 ms

bool bh1750Write(uint8_t cmd) {
  Wire.beginTransmission(BH1750_ADDR);
  Wire.write(cmd);
  return (Wire.endTransmission() == 0);
}

bool bh1750ReadRaw(uint16_t &raw) {
  // 2 Bytes Messwert abrufen
  if (Wire.requestFrom(BH1750_ADDR, (uint8_t)2) != 2) return false;
  uint8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  raw = ((uint16_t)msb << 8) | lsb;
  return true;
}

bool bh1750Begin() {
  // I2C starten
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(10);

  // Sensor wecken, resetten und Messmodus setzen
  if (!bh1750Write(BH1750_POWER_ON))  return false;
  if (!bh1750Write(BH1750_RESET))     return false;
  if (!bh1750Write(BH1750_CONT_HIRES1)) return false;
  return true;
}

float bh1750ReadLux() {
  // Im CONT_HIRES1-Modus liefert der Sensor kontinuierlich Messwerte.
  // Datenblatt-Faktor: Lux = raw / 1.2 (bei MTreg = 69).
  uint16_t raw;
  if (!bh1750ReadRaw(raw)) return NAN;
  return (float)raw / 1.2f;
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("\nBH1750 Lux-Messung (ESP32-C6)");

  // interne Pullups können helfen; bei längeren Leitungen externe 4.7k empfohlen
  pinMode(SDA_PIN, INPUT_PULLUP);
  pinMode(SCL_PIN, INPUT_PULLUP);

  if (!bh1750Begin()) {
    Serial.println("BH1750 Initialisierung fehlgeschlagen!");
    while (true) delay(1000);
  }
  Serial.println("BH1750 bereit.");
}

void loop() {
  float lux = bh1750ReadLux();
  if (isnan(lux)) {
    Serial.println("Fehler beim Lesen.");
  } else {
    Serial.print("Beleuchtungsstärke: ");
    Serial.print(lux, 1);
    Serial.println(" lx");
  }
  delay(250);  // schnelleres Update möglich; Sensor misst ~120 ms
}
