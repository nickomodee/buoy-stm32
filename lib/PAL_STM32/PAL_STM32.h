#pragma once

#include "PAL.h"
#include "PAL_STM32_UART.h" // PAL_STM32_UART_STREAM
#include "PAL_STM32_WIRE.h" // PAL_STM32_WIRE
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

#define PAL_STM32_MILLISECONDS HAL_GetTick
#define PAL_STM32_DELAY HAL_Delay