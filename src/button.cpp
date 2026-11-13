#include "button.h"

volatile bool Button::isrFlag_ = false;

// ISR мінімальний: лише ставить прапорець. IRAM_ATTR розміщує код у RAM,
// щоб переривання виконувалося навіть під час операцій з Flash.
void IRAM_ATTR Button::onFallingEdgeIsr() {
    isrFlag_ = true;
}
