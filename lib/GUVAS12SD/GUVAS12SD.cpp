#include "GUVAS12SD.h"

ADC_HandleTypeDef GUVAS12SD::hadc_ = {};

GUVAS12SD::GUVAS12SD(GPIO_TypeDef* GPIO_port, const uint8_t GPIO_pin, const uint32_t adc_channel) : GPIO_port_(GPIO_port), GPIO_pin_(GPIO_pin), adc_channel_(adc_channel) {}

bool GUVAS12SD::init() {
    GPIO_InitTypeDef GPIO_init = {};

    // GPIO Ports Clock Enable
    if (GPIO_port_ == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else {
        return false;
    }

    GPIO_init.Pin = GPIO_pin_;
    GPIO_init.Mode = GPIO_MODE_ANALOG;
    GPIO_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIO_port_, &GPIO_init);

    __HAL_RCC_ADC1_CLK_ENABLE();
    
    // completely reset ADC from when it was used for initialising the random seed
    __HAL_RCC_ADC1_FORCE_RESET();
    __HAL_RCC_ADC1_RELEASE_RESET();

    ADC_MultiModeTypeDef multimode = {};

    hadc_.Instance = ADC1;
    hadc_.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc_.Init.Resolution = ADC_RESOLUTION_12B;
    hadc_.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc_.Init.ContinuousConvMode = DISABLE;
    hadc_.Init.DiscontinuousConvMode = DISABLE;
    hadc_.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc_.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc_.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc_.Init.NbrOfConversion = 1;
    hadc_.Init.DMAContinuousRequests = DISABLE;
    hadc_.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc_.Init.LowPowerAutoWait = DISABLE;
    hadc_.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

    if (HAL_ADC_Init(&hadc_) != HAL_OK) {
        return false;
    }

    /** Configure the ADC multi-mode
     */
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc_, &multimode) != HAL_OK) {
        return false;
    }

    return HAL_ADCEx_Calibration_Start(&hadc_, ADC_SINGLE_ENDED) == HAL_OK;
}

float GUVAS12SD::read_uv_index() {
    constexpr uint16_t num_samples = 128;
    
    float voltage_sum = 0.0f;
    float compensation = 0.0f; // a running compensation for lost low-order bits

    for (uint16_t sample = 0; sample < num_samples; sample++) {
        float y = read_voltage() - compensation;
        float t = voltage_sum + y;
        compensation = (t - voltage_sum) - y;
        voltage_sum = t;
    }

    const float average_voltage = voltage_sum / (float)num_samples;

    return average_voltage;
}

float GUVAS12SD::ADC_to_voltage_(const uint32_t adc_value) {
    const uint32_t max_adc_value = (1 << ((uint32_t)ADC_RESOLUTION_)) - 1;
    return (adc_value / (float)max_adc_value) * ADC_REFERENCE_VOLTAGE_;
}

float GUVAS12SD::read_voltage() {
    const uint32_t adc_value = read_raw_adc_();
    return ADC_to_voltage_(adc_value);
}

uint32_t GUVAS12SD::read_raw_adc_() {
    if (!init_channel_()) {
        return 0.0f;
    }

    if (HAL_ADC_Start(&hadc_) != HAL_OK) {
        return 0.0f;
    }
    if (HAL_ADC_PollForConversion(&hadc_, 100) != HAL_OK) {
        return 0.0f;
    }
    const uint32_t value = HAL_ADC_GetValue(&hadc_);
    HAL_ADC_Stop(&hadc_);

    return value;
}

bool GUVAS12SD::init_channel_() {
    ADC_ChannelConfTypeDef channel_config = {};

    /** Configure Regular Channel
     */
    channel_config.Channel = adc_channel_;
    channel_config.Rank = ADC_REGULAR_RANK_1;
    channel_config.SingleDiff = ADC_SINGLE_ENDED;
    channel_config.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
    channel_config.OffsetNumber = ADC_OFFSET_NONE;
    channel_config.Offset = 0;
    return HAL_ADC_ConfigChannel(&hadc_, &channel_config) == HAL_OK;
}