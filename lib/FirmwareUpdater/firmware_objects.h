#pragma once

#include "firmware_update_linker.h"
#include "stm32f3xx_hal.h"
#include <cstdint>

/* ///////////////////////////////////////////////////////////////////////////
VERY IMPORTANT - ADD THESE TO THE HAL HEADER AND/OR THE RESPECTIVE SOURCE FILES:

`stm32f3xx_hal.c`:
`__weak uint32_t HAL_GetTick(void)`
`__weak void HAL_Delay(uint32_t Delay)`
`__IO __weak uint32_t uwTick;`
`__weak HAL_TickFreqTypeDef uwTickFreq = HAL_TICK_FREQ_DEFAULT;`

`system_stm32f3xx.c`
`__weak uint32_t SystemCoreClock = 8000000;`

`stm32f3xx_hal_gpio.c`:
`__weak void HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)`
`__weak void HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)`

`stm32f3xx_hal_spi.c`:
`__weak HAL_StatusTypeDef HAL_SPI_Init(SPI_HandleTypeDef *hspi)`
`__weak void __FIRMWARE HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)`
`__weak HAL_StatusTypeDef HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)`
`__weak HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,`
`__weak HAL_StatusTypeDef SPI_WaitFlagStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, FlagStatus State,`
`__weak HAL_StatusTypeDef SPI_WaitFifoStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Fifo, uint32_t State,`
`__weak HAL_StatusTypeDef SPI_EndRxTxTransaction(SPI_HandleTypeDef *hspi, uint32_t Timeout, uint32_t Tickstart)`
// also at the top of the file you need to modify the `static` declarations
`HAL_StatusTypeDef SPI_WaitFlagStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, FlagStatus State,`
`HAL_StatusTypeDef SPI_WaitFifoStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Fifo, uint32_t State,`
`HAL_StatusTypeDef SPI_EndRxTxTransaction(SPI_HandleTypeDef *hspi, uint32_t Timeout, uint32_t Tickstart);`

`core_cm4.h`: // comment these out
`extern void __NVIC_SystemReset(); // __NO_RETURN __STATIC_INLINE void __NVIC_SystemReset(void)`

`cmsis_gcc.h`: // comment these out
`extern void __enable_irq(); // __STATIC_FORCEINLINE void __enable_irq(void)`
`extern void __disable_irq(); // __STATIC_FORCEINLINE void __disable_irq(void)`
`extern uint32_t __get_PRIMASK(); // __STATIC_FORCEINLINE uint32_t __get_PRIMASK(void)`
`extern void __DSB(); // __STATIC_FORCEINLINE void __DSB(void)`

/////////////////////////////////////////////////////////////////////////// */

/* ///////////////////////////////////////////////////////////////////////////
VERY IMPORTANT - ADD THESE TO THE HAL HEADER AND/OR THE RESPECTIVE SOURCE FILES:

`stm32f3xx_hal.h/c`:
`#define __FIRMWARE __attribute((section(".firmware_update_section"))) __attribute__((used))`
`#define __FIRMWARE_BSS __attribute((section(".firmware_update_bss"))) __attribute__((used))`
`#define __FIRMWARE_DATA __attribute((section(".firmware_update_data"))) __attribute__((used))`
`#define __FIRMWARE_RODATA __attribute((section(".firmware_update_rodata"))) __attribute__((used))`
`uint32_t __FIRMWARE HAL_GetTick(void);`
`void __FIRMWARE HAL_Delay(uint32_t Delay);`
`extern __IO uint32_t __FIRMWARE_BSS uwTick;` && `__IO uint32_t __FIRMWARE_BSS uwTick;`
`extern HAL_TickFreqTypeDef __FIRMWARE_DATA uwTickFreq;` && `HAL_TickFreqTypeDef __FIRMWARE_DATA uwTickFreq = HAL_TICK_FREQ_DEFAULT;`

`system_stm32f3xx.h`
`#define __FIRMWARE __attribute((section(".firmware_update_section"))) __attribute__((used))`
`#define __FIRMWARE_BSS __attribute((section(".firmware_update_bss"))) __attribute__((used))`
`#define __FIRMWARE_DATA __attribute((section(".firmware_update_data"))) __attribute__((used))`
`#define __FIRMWARE_RODATA __attribute((section(".firmware_update_rodata"))) __attribute__((used))`
`extern uint32_t __FIRMWARE_DATA SystemCoreClock;` && `uint32_t __FIRMWARE_DATA SystemCoreClock = 8000000;`

`stm32f3xx_hal_gpio.h/c`:
`void __FIRMWARE HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)`
`void __FIRMWARE HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)`

`stm32f3xx_hal_spi.h/c`:
`HAL_StatusTypeDef __FIRMWARE HAL_SPI_Init(SPI_HandleTypeDef *hspi)`
`__weak void __FIRMWARE HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)`
`HAL_StatusTypeDef __FIRMWARE HAL_SPI_Transmit(SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout)`
`__FIRMWARE HAL_StatusTypeDef HAL_SPI_TransmitReceive(SPI_HandleTypeDef *hspi, uint8_t *pTxData, uint8_t *pRxData, uint16_t Size,`
`static HAL_StatusTypeDef __FIRMWARE SPI_WaitFlagStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Flag, FlagStatus State,`
`static HAL_StatusTypeDef __FIRMWARE SPI_WaitFifoStateUntilTimeout(SPI_HandleTypeDef *hspi, uint32_t Fifo, uint32_t State,`
`static HAL_StatusTypeDef __FIRMWARE SPI_EndRxTxTransaction(SPI_HandleTypeDef *hspi, uint32_t Timeout, uint32_t Tickstart)`

`core_cm4.h`
`#define __FIRMWARE __attribute((section(".firmware_update_section"))) __attribute__((used))`
`#define __FIRMWARE_BSS __attribute((section(".firmware_update_bss"))) __attribute__((used))`
`#define __FIRMWARE_DATA __attribute((section(".firmware_update_data"))) __attribute__((used))`
`#define __FIRMWARE_RODATA __attribute((section(".firmware_update_rodata"))) __attribute__((used))`
`__NO_RETURN __STATIC_INLINE void __FIRMWARE __NVIC_SystemReset(void)`

`cmsis_gcc.h`
`#define __FIRMWARE __attribute((section(".firmware_update_section"))) __attribute__((used))`
`#define __FIRMWARE_BSS __attribute((section(".firmware_update_bss"))) __attribute__((used))`
`#define __FIRMWARE_DATA __attribute((section(".firmware_update_data"))) __attribute__((used))`
`#define __FIRMWARE_RODATA __attribute((section(".firmware_update_rodata"))) __attribute__((used))`
`__STATIC_FORCEINLINE void __FIRMWARE __enable_irq(void)`
`__STATIC_FORCEINLINE void __FIRMWARE __disable_irq(void)`
`__STATIC_FORCEINLINE uint32_t __FIRMWARE __get_PRIMASK(void)`
`__STATIC_FORCEINLINE __FIRMWARE void __DSB(void)`

//////////////////////////////////////////////////////////////////////////// */