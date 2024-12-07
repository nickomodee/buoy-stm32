#pragma once

#include "../SD/SD.h"
#include "../FirmwareUpdater/firmware_update_linker.h"
#include <cstring>
#include <cstdint>

class SD_File {
    public:
        SD_File(const char* path) __FIRMWARE;
        ~SD_File() __FIRMWARE;
        bool open(const uint8_t mode) __FIRMWARE;
        bool close() __FIRMWARE;
        uint32_t write(const uint8_t* buffer, const size_t size) __FIRMWARE;
        uint32_t write(const char* buffer, const size_t size) __FIRMWARE;
        uint32_t write(const char* str) __FIRMWARE;
        bool write(const char data) __FIRMWARE;
        bool write(const uint8_t data) __FIRMWARE;
        uint32_t read(uint8_t* buffer, const size_t size) __FIRMWARE;
        uint32_t read(char* buffer, const size_t size) __FIRMWARE;
        int read() __FIRMWARE;
        uint32_t position() __FIRMWARE;
        bool seek(const uint32_t position) __FIRMWARE;
        uint32_t get_size() __FIRMWARE;
        bool remove() __FIRMWARE;
        bool rename(const char* new_path) __FIRMWARE;
        bool exists() __FIRMWARE;
    private:
        static constexpr size_t __FIRMWARE_RODATA MAX_PATH_SIZE = 32;

        FIL file_;
        uint8_t mode_;

        char path_[MAX_PATH_SIZE] = {0};
        bool open_ = false;
};