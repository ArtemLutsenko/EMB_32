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

void loop() {}