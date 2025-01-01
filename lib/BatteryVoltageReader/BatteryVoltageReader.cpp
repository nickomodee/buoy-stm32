#include "BatteryVoltageReader.h"

GPIO_TypeDef* BatteryVoltageReader::GPIO_port_ = GPIOA;
ADC_HandleTypeDef BatteryVoltageReader::hadc1_ = {};

BatteryVoltageReader::BatteryVoltageReader() {}

bool BatteryVoltageReader::init() {
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

    hadc1_.Instance = ADC1;
    hadc1_.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
    hadc1_.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1_.Init.ScanConvMode = ADC_SCAN_DISABLE;
    hadc1_.Init.ContinuousConvMode = DISABLE;
    hadc1_.Init.DiscontinuousConvMode = DISABLE;
    hadc1_.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1_.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1_.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1_.Init.NbrOfConversion = 1;
    hadc1_.Init.DMAContinuousRequests = DISABLE;
    hadc1_.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc1_.Init.LowPowerAutoWait = DISABLE;
    hadc1_.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

    if (HAL_ADC_Init(&hadc1_) != HAL_OK) {
        return false;
    }

    /** Configure the ADC multi-mode
     */
    multimode.Mode = ADC_MODE_INDEPENDENT;
    if (HAL_ADCEx_MultiModeConfigChannel(&hadc1_, &multimode) != HAL_OK) {
        return false;
    }

    return HAL_ADCEx_Calibration_Start(&hadc1_, ADC_SINGLE_ENDED) == HAL_OK;
}

bool BatteryVoltageReader::init_channel_() {
    ADC_ChannelConfTypeDef channel_config = {};

    /** Configure Regular Channel
     */
    channel_config.Channel = ADC_CHANNEL_;
    channel_config.Rank = ADC_REGULAR_RANK_1;
    channel_config.SingleDiff = ADC_SINGLE_ENDED;
    channel_config.SamplingTime = ADC_SAMPLETIME_601CYCLES_5;
    channel_config.OffsetNumber = ADC_OFFSET_NONE;
    channel_config.Offset = 0;
    return HAL_ADC_ConfigChannel(&hadc1_, &channel_config) == HAL_OK;
}

float BatteryVoltageReader::ADC_to_voltage_(const uint32_t adc_value) {
    const uint32_t max_adc_value = (1 << ((uint32_t)ADC_RESOLUTION_)) - 1;
    return (adc_value / (float)max_adc_value) * ADC_REFERENCE_VOLTAGE_;
}

uint32_t BatteryVoltageReader::read_raw_adc_() {
    if (!init_channel_()) {
        return 0.0f;
    }

    if (HAL_ADC_Start(&hadc1_) != HAL_OK) {
        return 0.0f;
    }
    if (HAL_ADC_PollForConversion(&hadc1_, 100) != HAL_OK) {
        return 0.0f;
    }
    const uint32_t value = HAL_ADC_GetValue(&hadc1_);
    HAL_ADC_Stop(&hadc1_);

    return value;
}

float BatteryVoltageReader::read_raw_voltage() {
    const uint32_t adc_value = read_raw_adc_();
    return ADC_to_voltage_(adc_value);
}

float BatteryVoltageReader::read_voltage() {
    return read_raw_voltage() * VOLTAGE_SCALER_;
}