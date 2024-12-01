#pragma once

#include "../SD/SD.h"
#include "../SD_File/SD_File.h"
#include "../PAL/PAL.h"

extern const char firmware_update_path[];
extern const char new_firmware_path[];
extern const char firmware_update_available_path[];
extern const char update_is_available_indicator_byte;

extern uint32_t _flash_update_start;
extern uint32_t _flash_update_end;

class FirmwareUpdater {
    public:
        FirmwareUpdater();
        static bool __FIRMWARE update();
        static bool __FIRMWARE check();
        static void __FIRMWARE mcu_reset();
        static void firmware_stream(const char* data, size_t size);
        static bool finish_firmware(const bool success);
    
    private:
        static bool __FIRMWARE flash_erase_page_(const uint32_t address);
        static bool __FIRMWARE flash_write_byte_(uint32_t address, uint16_t data);
        static bool __FIRMWARE begin_();
        static void __FIRMWARE end_();
        static bool initialise_firmware_();

        static uint32_t current_update_size_;
        static bool new_firmware_success_;
};