#pragma once

#include <cstdint>

// Уся конфігурація проєкту в одному місці.
// constexpr: значення обчислюються під час компіляції й не займають RAM.
// Магічних чисел у коді немає — лише іменовані константи звідси.

namespace config {

// Плата: ESP32-S3-DevKitM-1.
// GPIO17 і GPIO18 — звичайні виводи (не strapping: на S3 це 0, 3, 45, 46;
// не USB: GPIO19/20 зайняті USB D-/D+). Обидва поруч із GND на гребінці J1.

struct Led {
    static constexpr uint8_t  kPin              = 17;
    static constexpr uint32_t kBlinkIntervalMs  = 500;
};

struct Button {
    // Внутрішній pull-up: відпущена = HIGH, натиснута = LOW.
    static constexpr uint8_t  kPin              = 18;
    static constexpr uint32_t kDebounceMs       = 50;
};

struct SerialPort {
    static constexpr uint32_t kBaudRate         = 115200;
};

struct Profiler {
    // Звіт про час ітерації superloop кожні N ітерацій.
    static constexpr uint32_t kReportEveryIterations = 1000;
};

}  // namespace config
