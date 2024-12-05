#pragma once

#include "../SD/SD.h"
#include "../SD_File/SD_File.h"
#include "../PAL/PAL.h"
#include "../Debug/Debug.h"

extern const char __FIRMWARE_RODATA firmware_update_path[];
extern const char __FIRMWARE_RODATA new_firmware_path[];
extern const char __FIRMWARE_RODATA firmware_update_size_path[];
extern const char __FIRMWARE_RODATA firmware_update_checksum_path[];
extern const char __FIRMWARE_RODATA firmware_update_available_path[];
extern const char __FIRMWARE_RODATA update_is_available_indicator_byte;
extern const uint8_t __FIRMWARE_RODATA update_checksum_size;
extern const uint8_t __FIRMWARE_RODATA update_size_size;
extern const uint8_t __FIRMWARE_RODATA firmware_update_retries;

typedef uint16_t firmware_checksum_type;
typedef uint32_t firmware_size_type;

extern uint32_t _flash_update_start;
extern uint32_t _flash_update_end;

class FirmwareUpdater {
    public:
        FirmwareUpdater();
        static bool update();
        static bool check();
        static void firmware_stream(const uint8_t* data, const size_t size);
        static bool finish_firmware(const bool success);
        static bool initialise_firmware(const firmware_size_type expected_size, const firmware_checksum_type expected_checksum);
    
    private:
        static bool __FIRMWARE flash_erase_page_(const uint32_t address);
        static bool __FIRMWARE flash_write_byte_(uint32_t address, uint16_t data);
        static firmware_checksum_type __FIRMWARE calculate_flash_firmware_checksum_(const uint32_t start_flash_address, const uint32_t end_flash_address); // `flash_end_address` is the next byte after
        static void __FIRMWARE begin_();
        static void __FIRMWARE end_();
        static void __FIRMWARE mcu_reset_();

        static bool new_firmware_started_;
        static firmware_size_type new_firmware_expected_size_;
        static firmware_checksum_type new_firmware_expected_checksum_;
        static firmware_size_type current_update_size_;
        static firmware_checksum_type current_update_checksum_;
};