#pragma once

#include "fatfs.h"
#include "SD_conf.h"
#include "stm32f3xx_hal.h"
#include <cstring>

extern void Error_Handler();

class SD {
    public:
        SD();
        ~SD();
        bool begin();
        bool mount();
        bool unmount();
        bool open(FIL* file, const char* path, const uint8_t mode);
        bool close(FIL* file);
        uint32_t write(FIL* file, const uint8_t* buffer, const size_t size);
        uint32_t write(FIL* file, const char* buffer, const size_t size);
        uint32_t write(FIL* file, const char* str);
        bool write(FIL* file, const char data);
        bool write(FIL* file, const uint8_t data);
        uint32_t read(FIL* file, uint8_t* buffer, const size_t size);
        uint32_t read(FIL* file, char* buffer, const size_t size);
        int read(FIL* file);
        uint32_t position(FIL* file);
        bool seek(FIL* file, const uint32_t position);
        uint32_t file_size(FIL* file);
        uint32_t file_size();
        bool remove(const char* path);
        bool rmdir(const char* path);
        bool mkdir(const char* path);
        bool rename(const char* path, const char* new_path);
        bool exists(const char* path);
        bool is_dir(const char* path);
        bool is_dir();
        bool open_dir(const char* path);
        bool close_dir();
        bool next_file();
        size_t list_dir(const char* path, char* output, const size_t size);
        const char* file_name();
        uint16_t file_date();
        uint16_t file_time();
        uint32_t get_total_space();
        uint32_t get_available_space();
        SPI_HandleTypeDef* get_hspi_ptr();
    private:
        FATFS FatFs_;
        DIR dir_;
        bool dir_open_;
        bool mounted_;
        FILINFO file_info_;

        SPI_HandleTypeDef* hspi_;
        static const SPI_TypeDef* SPI_instance_;
};

extern SD sd;