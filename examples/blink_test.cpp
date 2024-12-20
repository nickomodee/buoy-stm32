#include "stm32f3xx_hal.h"
#include <cstdint>

// extern void blink(const uint32_t times);

void blink(const uint32_t times) {
    for (uint32_t i = 0; i < times; i++) {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
        HAL_Delay(500);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_3);
        HAL_Delay(500);
    }
}

void setup() {} // LED GPIO already initialised in `main_stm32.cpp`

void loop() {
    blink(1);
}