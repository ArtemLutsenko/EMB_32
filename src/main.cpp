#include <Arduino.h>

namespace Pins {
constexpr uint8_t RED = 6;
constexpr uint8_t YELLOW = 5;
constexpr uint8_t GREEN = 4;
constexpr uint8_t BUTTON = 7;
}

namespace Timing {
constexpr uint32_t GREEN = 10000;
constexpr uint32_t GREEN_BLINK = 3000;
constexpr uint32_t YELLOW = 2000;
constexpr uint32_t RED = 10000;
constexpr uint32_t RED_YELLOW = 2000;

constexpr uint32_t BLINK_INTERVAL = 500;
constexpr uint32_t DEBOUNCE = 50;
}

enum class Mode : uint8_t {
  Normal,
  FlashingYellow
};

enum class Phase : uint8_t {
  Green,
  GreenBlink,
  Yellow,
  Red,
  RedYellow
};

Mode mode = Mode::Normal;
Phase phase = Phase::Green;
uint32_t stateStartedAt = 0;

// Дані кнопки, спільні для переривання та loop().
portMUX_TYPE buttonMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool buttonRawLevel = HIGH;
volatile uint32_t buttonChangedAt = 0;

// Використовується лише основним кодом.
bool buttonStableLevel = HIGH;

// ---------- Переривання кнопки ----------

// Жодної логіки світлофора, затримок або Serial у перериванні.
void ARDUINO_ISR_ATTR onButtonChange() {
  portENTER_CRITICAL_ISR(&buttonMux);
  buttonRawLevel = digitalRead(Pins::BUTTON);
  buttonChangedAt = millis();
  portEXIT_CRITICAL_ISR(&buttonMux);
}

// true рівно один раз після стабільного натискання.
// Відпускання також має пройти перевірку стабільності.
bool buttonPressed() {
  bool pressed = false;

  portENTER_CRITICAL(&buttonMux);

  const uint32_t now = millis();
  const bool level = buttonRawLevel;

  if (now - buttonChangedAt >= Timing::DEBOUNCE &&
      level != buttonStableLevel) {
    buttonStableLevel = level;
    pressed = (level == LOW);
  }

  portEXIT_CRITICAL(&buttonMux);

  return pressed;
}

// ---------- Керування світлодіодами ----------

void setLights(bool red, bool yellow, bool green) {
  digitalWrite(Pins::RED, red ? HIGH : LOW);
  digitalWrite(Pins::YELLOW, yellow ? HIGH : LOW);
  digitalWrite(Pins::GREEN, green ? HIGH : LOW);
}

// ---------- Послідовність фаз ----------

uint32_t phaseDuration(Phase value) {
  switch (value) {
    case Phase::Green:      return Timing::GREEN;
    case Phase::GreenBlink: return Timing::GREEN_BLINK;
    case Phase::Yellow:     return Timing::YELLOW;
    case Phase::Red:        return Timing::RED;
    case Phase::RedYellow:  return Timing::RED_YELLOW;
  }

  return Timing::RED;
}

Phase nextPhase(Phase value) {
  switch (value) {
    case Phase::Green:      return Phase::GreenBlink;
    case Phase::GreenBlink: return Phase::Yellow;
    case Phase::Yellow:     return Phase::Red;
    case Phase::Red:        return Phase::RedYellow;
    case Phase::RedYellow:  return Phase::Green;
  }

  return Phase::Red;
}

void enterPhase(Phase next, uint32_t startedAt) {
  phase = next;
  stateStartedAt = startedAt;
}

// ---------- Перемикання режимів ----------

void toggleMode(uint32_t now) {
  if (mode == Mode::Normal) {
    mode = Mode::FlashingYellow;
    stateStartedAt = now;
  } else {
    mode = Mode::Normal;
    enterPhase(Phase::Green, now);
  }
}

// ---------- Оновлення стану ----------

void updateState(uint32_t now) {
  if (mode != Mode::Normal) {
    return;
  }

  // Відлік від планової межі фази запобігає накопиченню
  // похибки через невеликі затримки основного циклу.
  while (now - stateStartedAt >= phaseDuration(phase)) {
    const uint32_t nextStartedAt =
        stateStartedAt + phaseDuration(phase);

    enterPhase(nextPhase(phase), nextStartedAt);
  }
}

// ---------- Відображення стану ----------

// Світло повністю визначається режимом, фазою та часом.
// Окремі лічильники миготіння не потрібні.
void applyLights(uint32_t now) {
  const uint32_t elapsed = now - stateStartedAt;

  const bool firstHalf =
      ((elapsed / Timing::BLINK_INTERVAL) % 2) == 0;

  if (mode == Mode::FlashingYellow) {
    // Жовтий починає миготіти з увімкненого стану.
    setLights(false, firstHalf, false);
    return;
  }

  switch (phase) {
    case Phase::Green:
      setLights(false, false, true);
      break;

    case Phase::GreenBlink:
      // Починаємо з вимкнення після постійного зеленого.
      setLights(false, false, !firstHalf);
      break;

    case Phase::Yellow:
      setLights(false, true, false);
      break;

    case Phase::Red:
      setLights(true, false, false);
      break;

    case Phase::RedYellow:
      setLights(true, true, false);
      break;
  }
}

// ---------- Ініціалізація ----------

void setup() {
  pinMode(Pins::RED, OUTPUT);
  pinMode(Pins::YELLOW, OUTPUT);
  pinMode(Pins::GREEN, OUTPUT);
  pinMode(Pins::BUTTON, INPUT_PULLUP);

  attachInterrupt(
      digitalPinToInterrupt(Pins::BUTTON),
      onButtonChange,
      CHANGE
  );

  // Кнопка, затиснута під час запуску, не змінює режим.
  // Спочатку її потрібно відпустити.
  portENTER_CRITICAL(&buttonMux);
  buttonRawLevel = digitalRead(Pins::BUTTON);
  buttonChangedAt = millis();
  buttonStableLevel = buttonRawLevel;
  portEXIT_CRITICAL(&buttonMux);

  const uint32_t now = millis();
  enterPhase(Phase::Green, now);
  applyLights(now);
}

// ---------- Основний цикл ----------

void loop() {
  const bool pressed = buttonPressed();
  const uint32_t now = millis();

  if (pressed) {
    toggleMode(now);
  }

  updateState(now);
  applyLights(now);
}