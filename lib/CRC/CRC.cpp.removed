#include "CRC.h"

__FIRMWARE CRC16::CRC16() {
    __HAL_RCC_CRC_CLK_ENABLE();
    reset();
}

void __FIRMWARE CRC16::reset(const uint32_t start_crc) {
    CRC->INIT = start_crc;
    CRC->POL = (uint32_t)polynomial;
    CRC->CR = CRC_CR_RESET;

    CRC->CR = CRC_CR_POLYSIZE_0; // configure CRC for 16-bit
}

void __FIRMWARE CRC16::reset() {
    reset(0);
}

uint16_t __FIRMWARE CRC16::update(const uint8_t data) {
    *((uint8_t*)&CRC->DR) = data;
    return get_crc();
}

uint16_t __FIRMWARE CRC16::update(const uint8_t* data, const size_t data_size) {
    for (size_t i = 0; i < data_size; i++) {
        update(data[i]);
    }
    return get_crc();
}

uint16_t __FIRMWARE CRC16::update(const char* data, const size_t data_size) {
    return update((const uint8_t*)data, data_size);
}

uint16_t __FIRMWARE CRC16::get_crc() {
    return (uint16_t)CRC->DR;
}

__FIRMWARE CRC32::CRC32() {
    __HAL_RCC_CRC_CLK_ENABLE();
    reset();
}

void __FIRMWARE CRC32::reset(const uint32_t start_crc) {
    CRC->INIT = start_crc;
    CRC->POL = (uint32_t)polynomial;
    CRC->CR = CRC_CR_RESET;

    CRC->CR = 0; // configure CRC for 32-bit
}

void __FIRMWARE CRC32::reset() {
    reset(0);
}

uint32_t __FIRMWARE CRC32::update(const uint8_t data) {
    *((uint8_t*)&CRC->DR) = data;
    return get_crc();
}

uint32_t __FIRMWARE CRC32::update(const uint8_t* data, const size_t data_size) {
    for (size_t i = 0; i < data_size; i++) {
        update(data[i]);
    }
    return get_crc();
}

uint32_t __FIRMWARE CRC32::update(const char* data, const size_t data_size) {
    return update((const uint8_t*)data, data_size);
}

uint32_t __FIRMWARE __FIRMWARE CRC32::get_crc() {
    return (uint32_t)CRC->DR;
}

CRC16 __FIRMWARE_BSS crc16;
CRC32 __FIRMWARE_BSS crc32;