#pragma once

#include "SD.h"
#include <cstring>
#include <cstdint>

static constexpr size_t PATH_SIZE = 32;

class SD_File {
    public:
        SD_File(const char* path);
        ~SD_File();
        bool open(const uint8_t mode);
        bool close();
        uint32_t write(const uint8_t* buffer, const size_t size);
        uint32_t write(const char* buffer, const size_t size);
        uint32_t write(const char* str);
        bool write(const char data);
        bool write(const uint8_t data);
        uint32_t read(uint8_t* buffer, const size_t size);
        uint32_t read(char* buffer, const size_t size);
        int read();
        uint32_t position();
        bool seek(const uint32_t position);
        uint32_t get_size();
        bool remove();
        bool rename(const char* new_path);
        bool exists();
    private:
        FIL file_;
        uint8_t mode_;

        char path_[PATH_SIZE] = {0};
        bool open_ = false;
};