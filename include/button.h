#pragma once

#include <Arduino.h>
#include <cstdint>

// Кнопка на GPIO з перериванням.
//
// ISR лише ставить volatile-прапорець — жодних Serial.print, затримок
// чи логіки. Антидребезг і рішення "натискання було" — в poll(),
// який викликається з superloop.
//
// Антидребезг — маленький state machine без блокувань:
//   Idle         --(прапорець з ISR)-->            Debouncing
//   Debouncing   --(минуло kDebounceMs, пін LOW)-> WaitRelease  => подія
//   Debouncing   --(минуло kDebounceMs, пін HIGH)-> Idle        (це був шум)
//   WaitRelease  --(пін HIGH)-->                   Idle
class Button {
public:
    Button(uint8_t pin, uint32_t debounceMs) : pin_(pin), debounceMs_(debounceMs) {}

    void init() {
        pinMode(pin_, INPUT_PULLUP);
        attachInterrupt(digitalPinToInterrupt(pin_), &Button::onFallingEdgeIsr, FALLING);
    }

    // Повертає true рівно один раз на кожне підтверджене натискання.
    bool poll(uint32_t nowMs) {
        switch (state_) {
            case State::Idle:
                if (takeIsrFlag()) {
                    state_ = State::Debouncing;
                    edgeAtMs_ = nowMs;
                }
                return false;

            case State::Debouncing:
                if (nowMs - edgeAtMs_ < debounceMs_) {
                    return false;
                }
                if (isPressedNow()) {
                    state_ = State::WaitRelease;
                    return true;
                }
                state_ = State::Idle;
                return false;

            case State::WaitRelease:
                // Під час утримання та відпускання контакти теж брязкають —
                // ці фронти ігноруємо.
                takeIsrFlag();
                if (!isPressedNow()) {
                    state_ = State::Idle;
                }
                return false;
        }
        return false;
    }

private:
    enum class State : uint8_t { Idle, Debouncing, WaitRelease };

    // Єдина змінна, спільна для ISR і основного циклу.
    // volatile: компілятор зобов'язаний читати її з пам'яті при кожному
    // зверненні, а не кешувати в регістрі.
    static volatile bool isrFlag_;

    // Визначена в src/button.cpp: IRAM_ATTR-функція не може бути inline
    // у заголовку (інакше лінкер ESP32 видає "dangerous relocation").
    static void IRAM_ATTR onFallingEdgeIsr();

    static bool takeIsrFlag() {
        // Читання й скидання bool на ESP32 — окремі атомарні операції.
        // Якщо ISR спрацює між ними, ми втратимо лише фронт брязкоту,
        // що для кнопки нешкідливо.
        if (!isrFlag_) {
            return false;
        }
        isrFlag_ = false;
        return true;
    }

    bool isPressedNow() const { return digitalRead(pin_) == LOW; }

    const uint8_t pin_;
    const uint32_t debounceMs_;
    State state_ = State::Idle;
    uint32_t edgeAtMs_ = 0;
};
