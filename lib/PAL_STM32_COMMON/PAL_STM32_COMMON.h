#pragma once

// provide common utilities for use with `PAL_STM32` objects

#include "stm32f3xx_hal.h"
#include "../FirmwareUpdater/firmware_update_linker.h"

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)

// RAII
class STM32_INTERRUPT_GUARD {
    public:
        STM32_INTERRUPT_GUARD() __FIRMWARE;
        ~STM32_INTERRUPT_GUARD() __FIRMWARE;

    private:
        bool interruptsEnabled;
};

// For `PAL_SERIAL.print(n, base)`
#define PAL_OCT 8
#define PAL_DEC 10
#define PAL_HEX 16


constexpr uint8_t STM32_GET_PIN_NUMBER(uint16_t pin_mask) {
    return (uint8_t)__builtin_ctz(pin_mask); // trailing 0s
}

struct PinMapping {
    GPIO_TypeDef* GPIO_port;
    uint16_t GPIO_pin;
};

// ONLY for the Nucleo-F303k8
static constexpr PinMapping pin_map[] = {
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

static __attribute__((used)) GPIO_TypeDef* get_GPIO_port(const uint8_t physical_pin) {
    return pin_map[physical_pin].GPIO_port;
}

static __attribute__((used)) uint16_t get_GPIO_pin(const uint8_t physical_pin) {
    return pin_map[physical_pin].GPIO_pin;
}