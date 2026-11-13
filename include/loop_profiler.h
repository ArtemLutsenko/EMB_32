#pragma once

#include <Arduino.h>
#include <cstdint>

// Вимірює тривалість однієї ітерації superloop (у мікросекундах)
// і раз на N ітерацій друкує середнє / мінімум / максимум.
// Без динамічної пам'яті: лише кілька лічильників фіксованого розміру.
class LoopProfiler {
public:
    explicit constexpr LoopProfiler(uint32_t reportEvery) : reportEvery_(reportEvery) {}

    void begin() { startUs_ = micros(); }

    // Викликати в кінці корисної роботи ітерації.
    // Друк у Serial виконується вже після заміру, тому не потрапляє в статистику.
    void end() {
        const uint32_t elapsedUs = micros() - startUs_;
        totalUs_ += elapsedUs;
        if (elapsedUs < minUs_) minUs_ = elapsedUs;
        if (elapsedUs > maxUs_) maxUs_ = elapsedUs;

        if (++iterations_ >= reportEvery_) {
            report();
            reset();
        }
    }

private:
    void report() const {
        const float avgUs = static_cast<float>(totalUs_) / static_cast<float>(iterations_);
        Serial.printf("[loop] %lu iters: avg=%.2f us, min=%lu us, max=%lu us\n",
                      static_cast<unsigned long>(iterations_), avgUs,
                      static_cast<unsigned long>(minUs_),
                      static_cast<unsigned long>(maxUs_));
    }

    void reset() {
        iterations_ = 0;
        totalUs_ = 0;
        minUs_ = UINT32_MAX;
        maxUs_ = 0;
    }

    const uint32_t reportEvery_;
    uint32_t startUs_ = 0;
    uint32_t iterations_ = 0;
    uint64_t totalUs_ = 0;
    uint32_t minUs_ = UINT32_MAX;
    uint32_t maxUs_ = 0;
};
