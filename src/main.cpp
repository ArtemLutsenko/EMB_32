#include <Arduino.h>

// GPIO numbers, NOT physical header positions.
// Target: ESP32-S3 N16R8. Locate the GPIO labels on your actual board.
struct Config {
  static constexpr uint8_t ledPin = 18;
  static constexpr uint8_t buttonPin = 17;
  static constexpr uint32_t blinkIntervalMs = 500;
  static constexpr uint32_t debounceMs = 30;
};

enum class LedState : uint8_t { Off, On };
enum class Mode : uint8_t { Blink, AlwaysOn, AlwaysOff };

class Led {
 public:
  explicit constexpr Led(uint8_t pin) : pin_(pin) {}
  void init() {
    pinMode(pin_, OUTPUT);
    set(LedState::Off);
  }
  void set(LedState state) {
    digitalWrite(pin_, state == LedState::On ? HIGH : LOW);
  }
 private:
  const uint8_t pin_;
};

// The only global mutable objects: ISR event flag and its ESP32 spinlock.
// volatile alone does NOT synchronize an ISR and the application.
volatile bool button = false;
portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;

void ARDUINO_ISR_ATTR buttonISR() {
  portENTER_CRITICAL_ISR(&buttonMux);
  button = true;
  portEXIT_CRITICAL_ISR(&buttonMux);
}

bool takeButtonEvent() {
  portENTER_CRITICAL(&buttonMux);
  const bool pending = button;
  button = false;
  portEXIT_CRITICAL(&buttonMux);
  return pending;
}

class Application {
 public:
  void init() {
    led_.init();
    pinMode(Config::buttonPin, INPUT_PULLUP);
    rawLevel_ = digitalRead(Config::buttonPin);
    stableLevel_ = rawLevel_;
    changedAt_ = millis();
    lastToggle_ = changedAt_;
    attachInterrupt(Config::buttonPin, buttonISR, CHANGE);
  }

  void update() {
    const uint32_t now = millis();
    updateButton(now);
    if (mode_ == Mode::Blink &&
        static_cast<uint32_t>(now - lastToggle_) >= Config::blinkIntervalMs) {
      lastToggle_ = now;
      ledState_ = ledState_ == LedState::Off ? LedState::On : LedState::Off;
      led_.set(ledState_);
    }
  }

 private:
  void updateButton(uint32_t now) {
    const bool edgeSeen = takeButtonEvent();
    const int level = digitalRead(Config::buttonPin);
    // Restart the settling timer on every observed edge, including bounce.
    // Sampling also catches a level change around ISR installation.
    if (edgeSeen || level != rawLevel_) {
      rawLevel_ = level;
      changedAt_ = now;
    }
    if (rawLevel_ != stableLevel_ &&
        static_cast<uint32_t>(now - changedAt_) >= Config::debounceMs) {
      stableLevel_ = rawLevel_;
      if (stableLevel_ == LOW) nextMode(now);
    }
  }

  void nextMode(uint32_t now) {
    switch (mode_) {
      case Mode::Blink:
        mode_ = Mode::AlwaysOn;
        ledState_ = LedState::On;
        break;
      case Mode::AlwaysOn:
        mode_ = Mode::AlwaysOff;
        ledState_ = LedState::Off;
        break;
      case Mode::AlwaysOff:
        mode_ = Mode::Blink;
        ledState_ = LedState::Off;
        lastToggle_ = now;
        break;
    }
    led_.set(ledState_);
  }

  Led led_{Config::ledPin};
  LedState ledState_ = LedState::Off;
  Mode mode_ = Mode::Blink;
  int rawLevel_ = HIGH;
  int stableLevel_ = HIGH;
  uint32_t changedAt_ = 0;
  uint32_t lastToggle_ = 0;
};

Application& app() {
  static Application instance; // fixed storage; no new/malloc
  return instance;
}

void setup() { app().init(); }
void loop() { app().update(); }
