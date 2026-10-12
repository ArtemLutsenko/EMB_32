#include <Arduino.h>

#define RELAY_OUT 15
#define LDR_IN 4

#define SAMPLES 20
#define LIGHT_THRESHOLD 3000

#define ADC_MAX 4095.0
#define U_REF 3100.0

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_OUT, OUTPUT);
  pinMode(LDR_IN, INPUT);

  analogReadResolution(12);

  digitalWrite(RELAY_OUT, LOW);
}

void loop() {
  uint32_t sum = 0;

  for (int i = 0; i < SAMPLES; i++) {
    sum += analogRead(LDR_IN);
    delay(2);
  }

  float raw = sum / (float)SAMPLES;
  float voltage = raw * U_REF / ADC_MAX;

  bool isDark = raw < LIGHT_THRESHOLD;

  digitalWrite(RELAY_OUT, isDark ? HIGH : LOW);

  Serial.print("RAW: ");
  Serial.print(raw, 1);

  Serial.print(" | Voltage: ");
  Serial.print(voltage, 1);
  Serial.print(" mV");

  Serial.print(" | Light: ");
  Serial.print(isDark ? "DARK" : "BRIGHT");

  Serial.print(" | GPIO15: ");
  Serial.println(isDark ? 1 : 0);

  delay(500);
}