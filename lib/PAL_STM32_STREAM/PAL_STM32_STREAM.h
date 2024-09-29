#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include "stm32f3xx_hal.h"

class PAL_STM32_STREAM_BUFFER {
    public:
        static const uint16_t BUFFER_SIZE = 1024;

        PAL_STM32_STREAM_BUFFER();
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
        bool peek(uint8_t &data);
        uint16_t getCount() const;

    private:
        volatile uint8_t buffer_[BUFFER_SIZE];
        volatile uint16_t head_;
        volatile uint16_t tail_;
};

/**
 * @brief Similar to the Arduino `Stream` class.
 */
class PAL_STM32_STREAM {
    public:
        int available();
        int read();
        int peek();
        virtual size_t write(const uint8_t data) = 0;
        virtual size_t write(const char* str);
        virtual size_t write(const uint8_t* buffer, const size_t size);
        virtual size_t write(const char* buffer, const size_t size);

        /**
         * @brief Puts the received byte into the buffer.
         *
         * Uses `rx_byte_` to insert into the internal circular buffer.
         */
        void put_buffer();
        /**
         * @brief Get the rx byte ptr object.
         *
         * @warning This is const.
         * It should not be modified except for from interrupts (or other rx sources for the stream).
         * Modification may lead to rx data loss.
         * It should only be modified by interrupts (or other rx sources for the stream).
         * @returns A const volatile* pointer to the internal rx byte
         */
        const volatile uint8_t* get_rx_byte_ptr() const;

    protected:
        volatile uint8_t rx_byte_;

    private:
        PAL_STM32_STREAM_BUFFER UART_buffer_;
};