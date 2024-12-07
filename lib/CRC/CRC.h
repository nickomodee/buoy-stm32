#pragma once

#include "stm32f3xx_hal.h"
#include "../FirmwareUpdater/FirmwareUpdater.h"

// templates don't work for some reason because the methods aren't put into the specified sections

class CRC16 {
public:
    CRC16() __FIRMWARE;
    void reset(const uint32_t start_crc) __FIRMWARE;
    void reset() __FIRMWARE;
    uint16_t update(const uint8_t data) __FIRMWARE;
    uint16_t update(const uint8_t* data, const size_t data_size) __FIRMWARE;
    uint16_t update(const char* data, const size_t data_size) __FIRMWARE;
    uint16_t get_crc() __FIRMWARE;

private:
    static constexpr uint16_t __FIRMWARE_RODATA polynomial = 0xF13B;
};

class CRC32 {
public:
    CRC32() __FIRMWARE;
    void reset(const uint32_t start_crc) __FIRMWARE;
    void reset() __FIRMWARE;
    uint32_t update(const uint8_t data) __FIRMWARE;
    uint32_t update(const uint8_t* data, const size_t data_size) __FIRMWARE;
    uint32_t update(const char* data, const size_t data_size) __FIRMWARE;
    uint32_t get_crc() __FIRMWARE;

private:
    static constexpr uint32_t __FIRMWARE_RODATA polynomial = 0x741B8CD7;
};

extern CRC16 __FIRMWARE_BSS crc16;
extern CRC32 __FIRMWARE_BSS crc32;