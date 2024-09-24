#pragma once

#include <cstdint>
#include <cstdbool>
#include <cstring>
#include <cmath>

#include "stm32f3xx_hal.h"
// #include "stm32f3xx_hal_uart.h"
// #include "stm32f3xx_hal_cortex.h"
// #include "stm32f303x8.h"

// For `print(n, base)`
#define PAL_OCT 8
#define PAL_DEC 10
#define PAL_HEX 16

extern void Error_Handler(); // From main STM32 PAL

class PAL_STM32_UART_BUFFER {
    public:
        static const uint16_t BUFFER_SIZE = 1024;

        PAL_STM32_UART_BUFFER();
        void init();
        bool isEmpty() const;
        bool isFull() const;
        /**
         * @brief Puts the byte into the next position in the buffer.
         *
         * @warning This does not specially handle a full buffer and intentionally overflows the buffer, since it is a circular buffer.
         * If it overflows, the data starting from the tail of the buffer will be overwritten.
         * @param data
         */
        void put(const uint8_t data);
        bool get(uint8_t &data);
        uint16_t getCount() const;

    private:
        volatile uint8_t buffer_[BUFFER_SIZE];
        volatile uint16_t head_;
        volatile uint16_t tail_;
};

/**
 * @brief Similar to the Arduino `Stream` class.
 */
class PAL_STM32_UART_STREAM {
    public:
        PAL_STM32_UART_STREAM(USART_TypeDef* UART_instance);
        void begin(uint32_t baud_rate);
        int available();
        int read();
        size_t write(const uint8_t data);
        size_t write(const char* str);
        size_t write(const uint8_t* buffer, const size_t size);
        size_t write(const char* buffer, const size_t size);

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

        /**
         * @brief Puts the received byte from the UART interrupt into the buffer.
         *
         * Uses `rx_byte_` to insert into the internal circular buffer.
         */
        void put_buffer();
        /**
         * @brief Get the rx byte ptr object.
         *
         * @warning This is const.
         * It should not be modified.
         * Modification may lead to UART rx data loss.
         * It should only be modified by the UART interrupt.
         * @return const volatile*
         */
        const volatile uint8_t* get_rx_byte_ptr() const;
        UART_HandleTypeDef* get_huart_ptr();

    private:
        volatile UART_HandleTypeDef huart_;
        PAL_STM32_UART_BUFFER UART_buffer_;
        const USART_TypeDef* UART_instance_;
        volatile uint8_t rx_byte_;

        size_t printNumber(uint32_t n, uint8_t base);
        size_t printFloat(double number, uint8_t digits);
};