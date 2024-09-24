#include "PAL.h"

// Modified from: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WMath.cpp
void PAL_GENERAL_RANDOMSEED(uint32_t seed) {
    if(seed != 0) {
        srand(seed);
    }
}

// Modified from: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WMath.cpp
uint32_t PAL_GENERAL_RANDOM(uint32_t howbig) {
    if(howbig == 0) {
        return 0;
    }
    uint32_t val = rand();
    return val % howbig;
}

// Modified from: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/WMath.cpp
uint32_t PAL_GENERAL_RANDOM(uint32_t howsmall, uint32_t howbig) {
    if(howsmall >= howbig) {
        return howsmall;
    }
    uint32_t diff = howbig - howsmall;
    return PAL_GENERAL_RANDOM(diff) + howsmall;
}