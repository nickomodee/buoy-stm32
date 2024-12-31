#pragma once

#include "../fatfs/fatfs.h"
#include "SD_conf.h"
#include "stm32f3xx_hal.h"
#include "../FirmwareUpdater/firmware_update_linker.h"
#include "../PAL/PAL.h"
#include "../Watchdog/Watchdog.h"
#include <cstring>

// extern void Error_Handler();

class SD {
    public:
        SD() __FIRMWARE;
        ~SD() __FIRMWARE;
        bool begin() __FIRMWARE;
        bool mount() __FIRMWARE;
        bool unmount() __FIRMWARE;
        bool open(FIL* file, const char* path, const uint8_t mode) __FIRMWARE;
        bool close(FIL* file) __FIRMWARE;
        uint32_t write(FIL* file, const uint8_t* buffer, const size_t size) __FIRMWARE;
        uint32_t write(FIL* file, const char* buffer, const size_t size) __FIRMWARE;
        uint32_t write(FIL* file, const char* str) __FIRMWARE;
        bool write(FIL* file, const char data) __FIRMWARE;
        bool write(FIL* file, const uint8_t data) __FIRMWARE;
        uint32_t read(FIL* file, uint8_t* buffer, const size_t size) __FIRMWARE;
        uint32_t read(FIL* file, char* buffer, const size_t size) __FIRMWARE;
        int read(FIL* file) __FIRMWARE;
        uint32_t position(FIL* file) __FIRMWARE;
        bool seek(FIL* file, const uint32_t position) __FIRMWARE;
        uint32_t file_size(FIL* file) __FIRMWARE;
        uint32_t file_size() __FIRMWARE;
        bool remove(const char* path) __FIRMWARE;
        bool rmdir(const char* path) __FIRMWARE;
        bool mkdir(const char* path) __FIRMWARE;
        bool rename(const char* path, const char* new_path) __FIRMWARE;
        bool exists(const char* path) __FIRMWARE;
        bool is_dir(const char* path) __FIRMWARE;
        bool is_dir() __FIRMWARE;
        bool open_dir(const char* path) __FIRMWARE;
        bool close_dir() __FIRMWARE;
        bool next_file() __FIRMWARE;
        size_t list_dir(const char* path, char* output, const size_t size) __FIRMWARE;
        const char* file_name() __FIRMWARE;
        uint16_t file_date() __FIRMWARE;
        uint16_t file_time() __FIRMWARE;
        uint32_t get_total_space() __FIRMWARE;
        uint32_t get_available_space() __FIRMWARE;
        SPI_HandleTypeDef* get_hspi_ptr() __FIRMWARE;
    private:
        FATFS FatFs_;
        DIR dir_;
        bool dir_open_;
        bool mounted_;
        FILINFO file_info_;

        SPI_HandleTypeDef* hspi_;
        static const __FIRMWARE_DATA SPI_TypeDef* SPI_instance_;
};

extern SD __FIRMWARE_BSS sd;