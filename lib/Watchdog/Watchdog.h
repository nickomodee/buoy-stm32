#pragma once

#include "stm32f3xx_hal.h"
#include "../FirmwareUpdater/firmware_update_linker.h"

class Watchdog {
    public:
        __FIRMWARE Watchdog();

        bool __FIRMWARE init();
        void __FIRMWARE refresh();

    private:
        static constexpr uint32_t __FIRMWARE_RODATA IWDG_PRESCALER_ = IWDG_PRESCALER_256;
        static bool __FIRMWARE_DATA initialised_;
};

extern Watchdog __FIRMWARE_BSS watchdog;