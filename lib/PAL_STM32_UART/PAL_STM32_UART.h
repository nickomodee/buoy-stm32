#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstring>
#include <cmath>
#include "PAL_STM32_STREAM.h"

#include "stm32f3xx_hal.h"

// For `print(n, base)`
#define PAL_OCT 8
#define PAL_DEC 10
#define PAL_HEX 16

extern void Error_Handler(); // From main STM32 PAL

class PAL_STM32_UART_STREAM : public PAL_STM32_STREAM {
    public:
        PAL_STM32_UART_STREAM(USART_TypeDef* UART_instance);
        void begin(uint32_t baud_rate);
        size_t write(const uint8_t data) override;
        size_t write(const char* str) override;
        size_t write(const char* buffer, const size_t size) override;
        size_t write(const uint8_t* buffer, const size_t size) override; // We override so that we can transmit the entire buffer at once, instead of chunks like the base class does

        // PRINTS
        size_t print(const char* str);
        size_t print(const char n);
        size_t print(const uint8_t n, const uint8_t base = PAL_DEC);
        size_t print(const int n, const uint8_t base = PAL_DEC);
        size_t print(const unsigned int n, const uint8_t base = PAL_DEC);
        size_t print(const int32_t n, const uint8_t base = PAL_DEC);
        size_t print(const uint32_t n, const uint8_t base = PAL_DEC);
        size_t print(const double n, uint8_t digits = 2);

        size_t println(const char* str);
        size_t println(const char n);
        size_t println(const uint8_t n, const uint8_t base = PAL_DEC);
        size_t println(const int n, const uint8_t base = PAL_DEC);
        size_t println(const unsigned int n, const uint8_t base = PAL_DEC);
        size_t println(const int32_t n, const uint8_t base = PAL_DEC);
        size_t println(const uint32_t n, const uint8_t base = PAL_DEC);
        size_t println(const double n, uint8_t digits = 2);
        size_t println();

        UART_HandleTypeDef* get_huart_ptr();

    private:
        volatile UART_HandleTypeDef huart_;
        const USART_TypeDef* UART_instance_;

        size_t printNumber(uint32_t n, uint8_t base);
        size_t printFloat(double number, uint8_t digits);
};