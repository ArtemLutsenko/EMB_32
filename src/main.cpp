
#include <Arduino.h>

#define LDR_IN 4
#define LED_OUT 15

#define ADC_MAX 4095.0
#define U_REF 3100.0
#define LIGHT_THRESHOLD 1000

void setup() {
  Serial.begin(115200);

  pinMode(LDR_IN, INPUT);
  pinMode(LED_OUT, OUTPUT);

  analogReadResolution(12);
}

void loop() {
  int raw = analogRead(LDR_IN);

  float calculatedMv = raw * U_REF / ADC_MAX;

  int measuredMv = analogReadMilliVolts(LDR_IN);

  float error = 0;

  if (measuredMv != 0) {
    error = abs(calculatedMv - measuredMv)
            / measuredMv * 100.0;
  }

  Serial.print("RAW: ");
  Serial.print(raw);

  Serial.print(" | Calculated: ");
  Serial.print(calculatedMv, 2);
  Serial.print(" mV");

  Serial.print(" | Measured: ");
  Serial.print(measuredMv);
  Serial.print(" mV");

  Serial.print(" | Error: ");
  Serial.print(error, 2);
  Serial.println(" %");

  if (measuredMv < LIGHT_THRESHOLD) {
    digitalWrite(LED_OUT, HIGH);
  } else {
    digitalWrite(LED_OUT, LOW);
  }

  delay(100);
}
