#include <cstdint>

extern void blink(const uint32_t times);

void setup() {} // LED GPIO already initialised in `main_stm32.cpp`

void loop() {
    blink(1);
}