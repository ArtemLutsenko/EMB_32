#include <Arduino.h>

constexpr int LED_OUT_RED = 38;
constexpr int LED_OUT_BLUE = 40;

constexpr int INPUT_BOOT_KEY = 0;
constexpr int INPUT_EXTERNAL_BUTTON = 21;

enum class SpeedMode {
    SLOW,
    FAST
};

SpeedMode mode = SpeedMode::FAST;

void slowBlinkMode() {
    digitalWrite(LED_OUT_RED, HIGH);
    digitalWrite(LED_OUT_BLUE, LOW);
    delay(1000);

    digitalWrite(LED_OUT_RED, LOW);
    digitalWrite(LED_OUT_BLUE, HIGH);
    delay(1000);
}

void fastBlinkMode() {
    digitalWrite(LED_OUT_RED, HIGH);
    digitalWrite(LED_OUT_BLUE, HIGH);
    delay(200);

    digitalWrite(LED_OUT_RED, LOW);
    digitalWrite(LED_OUT_BLUE, LOW);
    delay(200);
}

void setup() {
    pinMode(LED_OUT_RED, OUTPUT);
    pinMode(LED_OUT_BLUE, OUTPUT);

    pinMode(INPUT_EXTERNAL_BUTTON, INPUT_PULLUP);
    pinMode(INPUT_BOOT_KEY, INPUT);
}

void loop() {

    if (digitalRead(INPUT_BOOT_KEY) == LOW) {
        mode = SpeedMode::SLOW;
    }

    if (digitalRead(INPUT_EXTERNAL_BUTTON) == LOW) {
        mode = SpeedMode::FAST;
    }

    if (mode == SpeedMode::FAST) {
        fastBlinkMode();
    } else {
        slowBlinkMode();
    }
}