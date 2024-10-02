/* ------------------ PLATFORM ABSTRACTION LAYER (PAL) ------------------ */
#pragma once

#include <cstdlib>
#include <cstdint>
#include <stdio.h>

#define PLATFORM STM32

#if PLATFORM == STM32
#include "PAL_STM32.h"

#else
#include "PAL_ARDUINO.h"

#endif

// These are in "PAL_STM32_UART.h" or "PAL_ARDUINO.h"
// #define PAL_OCT 8
// #define PAL_DEC 10
// #define PAL_HEX 16

#define PAL_GENERAL_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PAL_GENERAL_MAX(a, b) ((a) > (b) ? (a) : (b))

#define PAL_MIN PAL_GENERAL_MIN
#define PAL_MAX PAL_GENERAL_MAX

extern void PAL_GENERAL_RANDOMSEED(uint32_t seed);
extern uint32_t PAL_GENERAL_RANDOM(uint32_t howbig);
extern uint32_t PAL_GENERAL_RANDOM(uint32_t howsmall, uint32_t howbig);

#define CONCAT(a, b, c, d, e) a##b##c##d##e // concatenations into 'PAL_{PLATFORM}_{func}'
#define EXPAND_CONCAT(a, b, c, d, e) CONCAT(a, b, c, d, e) // this forces expansion of PLATFORM
#define PAL_EXPAND(x) EXPAND_CONCAT(PAL, _, PLATFORM, _, x)

#define PAL_RANDOMSEED PAL_GENERAL_RANDOMSEED
#define PAL_RANDOM PAL_GENERAL_RANDOM
#define PAL_RANDOMSEED_INIT_ENTROPY PAL_EXPAND(RANDOMSEED_INIT_ENTROPY)

#define PAL_STREAM PAL_EXPAND(STREAM)
#define PAL_GENERAL_SERIAL Serial
#define PAL_SERIAL PAL_GENERAL_SERIAL
#define PAL_SERIAL PAL_GENERAL_SERIAL

#define PAL_MILLISECONDS PAL_EXPAND(MILLISECONDS)
#define PAL_DELAY PAL_EXPAND(DELAY)

#define PAL_GENERAL_WIRE Wire
#define PAL_WIRE PAL_GENERAL_WIRE
#define PAL_TWOWIRE PAL_EXPAND(TWOWIRE)