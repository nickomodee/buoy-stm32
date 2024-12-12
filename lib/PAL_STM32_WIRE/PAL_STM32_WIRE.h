#pragma once

#include "../PAL_STM32_STREAM/PAL_STM32_STREAM.h"
#include <cstdint>
#include <cstdbool>
#include <cstddef>
#include <cstring>

extern void Error_Handler();

#define WIRE_MAX_TIMEOUT 1000 // 1 second for the timeout for HAL Wire events

// Modified from: https://github.com/arduino/ArduinoCore-avr/blob/master/libraries/Wire/src/Wire.h and https://github.com/arduino/ArduinoCore-avr/blob/master/libraries/Wire/src/Wire.cpp
class PAL_STM32_WIRE : public PAL_STM32_STREAM {
    public:
        static const uint16_t RX_BUFFER_SIZE = 256;
        static const uint16_t TX_BUFFER_SIZE = 256;
        PAL_STM32_WIRE();
        void begin();
        void beginTransmission(const uint8_t address);
        uint8_t endTransmission();
        uint8_t endTransmission(const bool send_stop);
        size_t requestFrom(const uint8_t address, size_t size);
        size_t requestFrom(const uint8_t address, size_t size, const bool send_stop);
        void flush();
        size_t write(const uint8_t data) override;
        using PAL_STM32_STREAM::write; // don't override the virtual `write()` methods

        static I2C_HandleTypeDef* get_hi2c_ptr();
    private:
        static uint8_t tx_address_;
        static bool transmitting_;
        static PAL_STM32_STREAM_BUFFER tx_buffer_;
        static const I2C_TypeDef* I2C_instance_;
        static volatile I2C_HandleTypeDef hi2c_;
};