// Модуль 2.1 — Embedded C++ на ESP32
//
//  * неблокуючий blink через millis() (без delay)
//  * enum class для стану LED і режиму роботи
//  * constexpr-конфігурація без магічних чисел (include/config.h)
//  * кнопка на перериванні: ISR ставить volatile-прапорець,
//    антидребезг і перемикання режимів — у superloop
//  * вимірювання часу ітерації loop() зі звітом кожні 1000 ітерацій
//  * без new/malloc і без контейнерів STL

#include <Arduino.h>

#include "button.h"
#include "config.h"
#include "led.h"
#include "loop_profiler.h"

namespace {

enum class LedMode : uint8_t { Blink, AlwaysOn, AlwaysOff };

constexpr LedMode nextMode(LedMode mode) {
    switch (mode) {
        case LedMode::Blink:     return LedMode::AlwaysOn;
        case LedMode::AlwaysOn:  return LedMode::AlwaysOff;
        case LedMode::AlwaysOff: return LedMode::Blink;
    }
    return LedMode::Blink;
}

// Перевірка під час компіляції: цикл режимів замкнений.
static_assert(nextMode(nextMode(nextMode(LedMode::Blink))) == LedMode::Blink,
              "Mode cycle must be Blink -> On -> Off -> Blink");

constexpr const char* modeName(LedMode mode) {
    switch (mode) {
        case LedMode::Blink:     return "Blink";
        case LedMode::AlwaysOn:  return "AlwaysOn";
        case LedMode::AlwaysOff: return "AlwaysOff";
    }
    return "?";
}

// Увесь стан програми — в одному об'єкті, а не в розкиданих глобальних змінних.
// Об'єкти створюються статично (без heap), розмір відомий під час компіляції.
class App {
public:
    void init() {
        Serial.begin(config::SerialPort::kBaudRate);
        led_.init();
        button_.init();
        lastToggleMs_ = millis();
        Serial.printf("Started. Mode: %s\n", modeName(mode_));
    }

    // Одна ітерація superloop. Кожен крок короткий і ніколи не блокує.
    void tick() {
        profiler_.begin();

        const uint32_t nowMs = millis();
        handleButton(nowMs);
        updateLed(nowMs);

        profiler_.end();
    }

private:
    void handleButton(uint32_t nowMs) {
        if (!button_.poll(nowMs)) {
            return;
        }
        mode_ = nextMode(mode_);
        lastToggleMs_ = nowMs;  // блимання після перемикання починається з нового відліку
        Serial.printf("Button -> mode: %s\n", modeName(mode_));
    }

    void updateLed(uint32_t nowMs) {
        switch (mode_) {
            case LedMode::AlwaysOn:
                led_.set(LedState::On);
                break;

            case LedMode::AlwaysOff:
                led_.set(LedState::Off);
                break;

            case LedMode::Blink:
                // Віднімання беззнакових чисел коректне й при переповненні
                // millis() (~49.7 доби).
                if (nowMs - lastToggleMs_ >= config::Led::kBlinkIntervalMs) {
                    lastToggleMs_ += config::Led::kBlinkIntervalMs;
                    led_.toggle();
                }
                break;
        }
    }

    Led led_{config::Led::kPin};
    Button button_{config::Button::kPin, config::Button::kDebounceMs};
    LoopProfiler profiler_{config::Profiler::kReportEveryIterations};
    LedMode mode_ = LedMode::Blink;
    uint32_t lastToggleMs_ = 0;
};

// Єдиний екземпляр застосунку. Видимий лише в цьому файлі (анонімний namespace),
// щоб setup() і loop() Arduino мали до нього доступ.
App app;

}  // namespace

void setup() {
    app.init();
}

void loop() {
    app.tick();
}
