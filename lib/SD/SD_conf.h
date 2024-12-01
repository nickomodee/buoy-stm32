#pragma once

#include "stm32f3xx_hal.h"
#include "../FirmwareUpdater/firmware_update_linker.h"
// #define __FIRMWARE_BSS __attribute((section(".firmware_update_bss"))) __attribute__((used))
extern SPI_HandleTypeDef __FIRMWARE_BSS hspi;

#define SD_CS_Pin GPIO_PIN_1
#define SD_CS_GPIO_Port GPIOB
#define SD_SPI_HANDLE hspi