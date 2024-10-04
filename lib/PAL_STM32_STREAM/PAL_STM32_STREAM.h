#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include "stm32f3xx_hal.h"

#ifndef STM32_NO_INTERRUPTS
    #define STM32_NO_INTERRUPTS() const bool was_irq_enabled = ~(__get_PRIMASK() & 1);\
                                  if (was_irq_enabled) { \
                                      __disable_irq(); \
                                  }
#endif
#ifndef STM32_INTERRUPTS
    #define STM32_INTERRUPTS() if (was_irq_enabled) { \
                                   __enable_irq(); \
                               }
#endif

class PAL_STM32_STREAM_BUFFER {
    public:
        PAL_STM32_STREAM_BUFFER(const uint16_t buffer_size);
        ~PAL_STM32_STREAM_BUFFER();
        void init();
        bool is_empty() const;
        bool is_full() const;
        /**
         * @brief Puts the byte into the next position in the buffer.
         *
         * @warning This does not specially handle a full buffer and intentionally overflows the buffer, since it is a circular buffer.
         * If it overflows, the data starting from the tail of the buffer will be overwritten.
         * @param data
         */
        void empty();
        void reset();
        void put(const uint8_t data);
        bool get(uint8_t &data);
        bool peek(uint8_t &data);
        const uint8_t* get_read_ptr();
        uint16_t get_count() const;
        void set_count(const uint16_t count);

        const uint16_t BUFFER_SIZE;

    private:
        volatile uint8_t* buffer_;
        volatile uint16_t head_;
        volatile uint16_t tail_;
};

/**
 * @brief Similar to the Arduino `Stream` class.
 */
class PAL_STM32_STREAM {
    public:
        const uint16_t RX_BUFFER_SIZE;
        PAL_STM32_STREAM(const uint16_t rx_buffer_size) : RX_BUFFER_SIZE(rx_buffer_size), rx_buffer_(rx_buffer_size) {};

        int available();
        int read();
        int peek();
        virtual size_t write(const uint8_t data) = 0;
        virtual size_t write(const char* str);
        virtual size_t write(const uint8_t* buffer, const size_t size);
        virtual size_t write(const char* buffer, const size_t size);

        /**
         * @brief Puts the byte into the buffer.
         */
        void put_buffer(const uint8_t data);
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
        const uint8_t get_rx_byte() const;
        virtual ~PAL_STM32_STREAM() = default;

    protected:
        volatile uint8_t rx_byte_;
        PAL_STM32_STREAM_BUFFER rx_buffer_;
};