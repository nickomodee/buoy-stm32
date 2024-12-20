#include "PAL_STM32.h"

PAL_STM32_UART_STREAM Serial(USART2);
PAL_STM32_WIRE Wire;

static ADC_HandleTypeDef hadc1;

// Modified from: https://visualgdb.com/tutorials/arm/stm32/adc/
static void configure_ADC() {
    __HAL_RCC_ADC1_CLK_ENABLE();
 
    ADC_ChannelConfTypeDef adc_channel;
 
    hadc1.Instance = ADC1;
 
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV1;  // faster ADC clock for more noise
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE; // irregular sampling for more noise
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
 
    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        Error_Handler();
    }
 
    adc_channel.Channel = ADC_CHANNEL_VBAT; // we can use VREFINT, TEMPSENSOR, VBAT (seems the most noisy since I believe it is floating idrk docs are confusing)
    adc_channel.Rank = 1;
    adc_channel.Rank = ADC_REGULAR_RANK_1;
    adc_channel.SamplingTime = ADC_SAMPLETIME_1CYCLE_5; // shortest sampling time to increase volatility for more noise
    adc_channel.SingleDiff = ADC_SINGLE_ENDED;
    adc_channel.OffsetNumber = ADC_OFFSET_NONE;
    adc_channel.Offset = 0;
 
    if (HAL_ADC_ConfigChannel(&hadc1, &adc_channel) != HAL_OK) {
        Error_Handler();
    }
}

static bool is_adc_initialised = false;
static void init_adc_for_entropy() {
    if (is_adc_initialised) {
        return;
    }

    configure_ADC();

    is_adc_initialised = true;
}

static uint32_t get_adc_random_value() {
    uint32_t seed = 0;

    for (int i = 0; i < 32; i++) {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
            uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
            seed ^= (adc_value & 0x1) << i; // Use only the least significant bit of each sample
        }
        HAL_ADC_Stop(&hadc1);
    }

    return seed;
}

// static uint32_t get_micros_random_value() {
//     uint32_t seed = 0;

//     for (int i = 0; i < 32; i++) {
//         const uint32_t micros = PAL_STM32_MICROS();
//         seed ^= (micros & 0x1) << i; // Use only the least significant bit of each sample
//     }

//     return seed;
// }

uint32_t PAL_STM32_RANDOMSEED_INIT_ENTROPY() {
    init_adc_for_entropy();

    const uint32_t seed = get_adc_random_value();
    // const uint32_t seed = get_micros_random_value();
    PAL_GENERAL_RANDOMSEED(seed);
    return seed;
}


static bool is_dwt_initialised = false;

uint32_t PAL_STM32_MICROS() {
    if (!is_dwt_initialised) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // enable DWT
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // enable cycle counter
        is_dwt_initialised = true;
    }
    
    const uint32_t ticks = DWT->CYCCNT;
    return ticks / (SystemCoreClock / 1000000);
}

void PAL_STM32_DELAY_US(const uint32_t us) {
    if (!is_dwt_initialised) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // enable DWT
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // enable cycle counter
        is_dwt_initialised = true;
    }
    
    const uint32_t start = DWT->CYCCNT;
    const uint32_t ticks = us * (SystemCoreClock / 1000000); // convert us to clock ticks
    while ((DWT->CYCCNT - start) < ticks) {};
}

void PAL_STM32_SLEEP() {
    // // disaple peripherals

    // // disable ADC
    // ADC1->CR |= ADC_CR_ADDIS; 
    // ADC2->CR |= ADC_CR_ADDIS;
    
    // // disable DAC
    // DAC->CR &= ~DAC_CR_EN1;
    // DAC->CR &= ~DAC_CR_EN2;

    // // disable GPIO clocks
    // RCC->AHBENR &= ~(RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOFEN);

    // // disable USART, SPI, I2C clocks
    // RCC->APB1ENR &= ~(RCC_APB1ENR_USART2EN | RCC_APB1ENR_I2C1EN);
    // RCC->APB2ENR &= ~(RCC_APB2ENR_USART1EN | RCC_APB2ENR_SPI1EN);

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    DBGMCU->CR = 0; // disable debug interface
    HAL_PWR_EnterSTANDBYMode();
}