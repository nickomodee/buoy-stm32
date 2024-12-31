#include "Watchdog.h"

bool Watchdog::initialised_ = false;

__FIRMWARE Watchdog::Watchdog() {}

bool __FIRMWARE Watchdog::init() {
    if (initialised_) { // we can't init more than once
        return false;
    }

    IWDG->KR = 0xCCCC; // key to enable the watchdog peripheral
    IWDG->KR = 0x5555; // enable write access for PR and RLR
    IWDG->PR = IWDG_PRESCALER_; // set prescaler value
    IWDG->RLR = 0xFFF; // set reload register value to max
    IWDG->KR = 0xAAAA; // key to reload the watchdog counter to start it

    const uint32_t clock_frequency = SystemCoreClock / 1000; // ticks per ms
    const uint32_t timeout = 100; // in ms
    const uint32_t max_iterations = clock_frequency * timeout;

    for (uint32_t i = 0; i < max_iterations; i++) {
        if ((IWDG->SR & (IWDG_SR_WVU | IWDG_SR_RVU | IWDG_SR_PVU)) == 0) {
            initialised_ = true;
            
            return true; // success if the status register clears
        }
    }

    return false;
}

void __FIRMWARE Watchdog::refresh() {
    if (!initialised_) {
        return;
    }

    IWDG->KR = 0xAAAA; // key to reload the watchdog counter
}

Watchdog __FIRMWARE_BSS watchdog;