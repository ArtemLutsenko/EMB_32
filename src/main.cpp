#include <Arduino.h>

constexpr int LED_OUT_RED = 38;
constexpr int LED_OUT_BLUE = 40;
constexpr int INPUT_BOOT_KEY = 0;
constexpr int INPUT_EXTERNAL_BUTTON = 21;

constexpr int FAST_DELAY = 200;
constexpr int SLOW_DELAY = 1000;

enum class SpeedMode {
    SLOW,
    FAST
};

SpeedMode mode = SpeedMode::FAST;

void slowBlinkMode() {
    digitalWrite(LED_OUT_RED, HIGH);
    digitalWrite(LED_OUT_BLUE, LOW);
    delay(SLOW_DELAY);

    digitalWrite(LED_OUT_RED, LOW);
    digitalWrite(LED_OUT_BLUE, HIGH);
    delay(SLOW_DELAY);
}

void fastBlinkMode() {
    digitalWrite(LED_OUT_RED, HIGH);
    digitalWrite(LED_OUT_BLUE, HIGH);
    delay(FAST_DELAY);

    digitalWrite(LED_OUT_RED, LOW);
    digitalWrite(LED_OUT_BLUE, LOW);
    delay(FAST_DELAY);
}

void setup() {
    pinMode(LED_OUT_RED, OUTPUT);
    pinMode(LED_OUT_BLUE, OUTPUT);

    pinMode(INPUT_BOOT_KEY, INPUT_PULLUP);
    pinMode(INPUT_EXTERNAL_BUTTON, INPUT_PULLUP);
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