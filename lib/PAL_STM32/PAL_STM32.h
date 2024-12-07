#pragma once

#include "../PAL/PAL.h"
#include "../PAL_STM32_UART/PAL_STM32_UART.h" // PAL_STM32_UART_STREAM
#include "../PAL_STM32_WIRE/PAL_STM32_WIRE.h" // PAL_STM32_WIRE
#include "../PAL_STM32_ONEWIRE/PAL_STM32_ONEWIRE.h" // PAL_STM32_ONEWIRE
#include <cstdbool>
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_adc.h"

// #define PROGMEM __attribute__((section(".rodata")))
#define PROGMEM
#define F(x) ((const char*)(x))

extern uint32_t PAL_STM32_RANDOMSEED_INIT_ENTROPY();

extern PAL_STM32_UART_STREAM Serial;
extern PAL_STM32_WIRE Wire;

#define PAL_STM32_TWOWIRE PAL_STM32_WIRE

#define PAL_STM32_ONEWIRE PAL_STM32_ONEWIRE

#define PAL_STM32_MILLISECONDS HAL_GetTick
#define PAL_STM32_DELAY HAL_Delay
extern void PAL_STM32_DELAY_US(const uint32_t us);

extern void PAL_STM32_SLEEP();