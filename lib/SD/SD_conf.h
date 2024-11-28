#pragma once

#include "stm32f3xx_hal.h"

extern SPI_HandleTypeDef hspi;

#define SD_CS_Pin GPIO_PIN_1
#define SD_CS_GPIO_Port GPIOB
#define SD_SPI_HANDLE hspi