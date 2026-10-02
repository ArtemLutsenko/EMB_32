#include <Arduino.h>

#define LED_OUT_RED 37
#define LED_OUT_BLUE 40

void setup() {
    pinMode(LED_OUT_RED, OUTPUT);
    pinMode(LED_OUT_BLUE, OUTPUT);
}

void loop() {
    digitalWrite(LED_OUT_BLUE, LOW);
    digitalWrite(LED_OUT_RED, HIGH);
    delay(1000);

    digitalWrite(LED_OUT_RED, LOW);
    digitalWrite(LED_OUT_BLUE, HIGH);
    
    delay(1000);

}