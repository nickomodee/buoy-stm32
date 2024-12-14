#include "PAL_STM32_COMMON.h"

__FIRMWARE STM32_INTERRUPT_GUARD::STM32_INTERRUPT_GUARD() : interruptsEnabled(!(__get_PRIMASK())) {
    if (interruptsEnabled) {
        __disable_irq();
    }
}

__FIRMWARE STM32_INTERRUPT_GUARD::~STM32_INTERRUPT_GUARD() {
    if (interruptsEnabled) {
        __enable_irq();
    }
}