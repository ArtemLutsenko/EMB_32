#include <Arduino.h>

#define LDR_IN 4
#define LED_OUT 15

void setup() {
  pinMode(LED_OUT, OUTPUT);
}

void loop() {
  int measuredMv = analogReadMilliVolts(LDR_IN);

  if (measuredMv < 1000) {
    digitalWrite(LED_OUT, HIGH);
  } else {
    digitalWrite(LED_OUT, LOW);
  }

  delay(100);
}

