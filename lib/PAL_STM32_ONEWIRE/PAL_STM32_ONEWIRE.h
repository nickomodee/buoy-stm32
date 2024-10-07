#pragma once

#include <cstdint>
#include "stm32f3xx_hal.h"
#include <cstring>

extern void PAL_STM32_DELAY_US(const uint32_t us); // we can't import `PAL_STM32.h` due to circular imports

#define GET_PIN_NUMBER(pin_mask) (__builtin_ctz(pin_mask)) // trailing 0s

struct PinMapping {
    GPIO_TypeDef* GPIO_port;
    uint16_t GPIO_pin;
};

// ONLY for the Nucleo-F303k8
static const PinMapping pin_map[] = {
    // Digital pins
    {GPIOA, GPIO_PIN_10},  // D0
    {GPIOA, GPIO_PIN_9},  // D1
    {GPIOA, GPIO_PIN_12}, // D2
    {GPIOB, GPIO_PIN_0},  // D3
    {GPIOB, GPIO_PIN_7},  // D4
    {GPIOB, GPIO_PIN_6},  // D5
    {GPIOB, GPIO_PIN_1},  // D6
    {GPIOF, GPIO_PIN_0},  // D7
    {GPIOF, GPIO_PIN_1},  // D8
    {GPIOA, GPIO_PIN_8},  // D9
    {GPIOA, GPIO_PIN_11},  // D10
    {GPIOB, GPIO_PIN_5}, // D11
    {GPIOB, GPIO_PIN_4}, // D12
    {GPIOB, GPIO_PIN_3},  // D13

    // Analog pins
    {GPIOA, GPIO_PIN_0},  // A0
    {GPIOA, GPIO_PIN_1},  // A1
    {GPIOA, GPIO_PIN_3},  // A2
    {GPIOA, GPIO_PIN_4},  // A3
    {GPIOA, GPIO_PIN_5},  // A4
    {GPIOA, GPIO_PIN_6},  // A5
    {GPIOA, GPIO_PIN_7},  // A6
    {GPIOA, GPIO_PIN_2},  // A7
};

// values
#define ONEWIRE_CHOOSE_ROM (0x55)
#define ONEWIRE_SKIP_ROM (0xCC)
#define ONEWIRE_NORMAL_SEARCH (0xF0)
#define ONEWIRE_CONDITIONAL_SEARCH (0xEC)

// Modified from: https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.h and https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp
class PAL_STM32_ONEWIRE {
    public:
        PAL_STM32_ONEWIRE();
        PAL_STM32_ONEWIRE(const uint8_t physical_pin); // initialise with the board's physical pin number (Arduino Nano format). STM32 typically uses separate GPIO port and GPIO pin.
        void begin(const uint8_t physical_pin); // initialise with the board's physical pin number (Arduino Nano format). STM32 typically uses separate port and pin.
        bool reset();
        void select(const uint8_t rom[8]);
        void skip();
        void write(const uint8_t value, const bool power = false);
        void write_bytes(const uint8_t* buffer, const uint16_t buffer_size, const bool power = false);
        uint8_t read();
        void read_bytes(uint8_t* buffer, const uint16_t buffer_size);
        void write_bit(const uint8_t value);
        uint8_t read_bit();
        void depower();
        void reset_search();
        void target_search(const uint8_t family_code);
        bool search(uint8_t* new_address, const bool search_mode = true);
        static uint8_t crc8(const uint8_t* address, const uint8_t length);
        static bool check_crc16(const uint8_t* input, const uint16_t length, const uint8_t* inverted_crc, uint16_t crc = 0);
        static uint16_t crc16(const uint8_t* input, const uint16_t length, uint16_t crc = 0);
    private:
        GPIO_InitTypeDef GPIO_init_;
        GPIO_TypeDef* GPIO_port_;
        uint16_t GPIO_pin_;

        // search state
        unsigned char ROM_NO[8];
        uint8_t last_discrepancy;
        uint8_t last_family_discrepancy;
        bool last_device_flag;

        void set_input();
        void set_output();
        bool read_pin();
        void write_pin(const GPIO_PinState pin_state);
};