#pragma once

#include "stm32f3xx_hal.h"
#include "../FirmwareUpdater/firmware_update_linker.h"

extern SPI_HandleTypeDef __FIRMWARE_BSS hspi;

#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOA
#define SD_SPI_HANDLE hspi