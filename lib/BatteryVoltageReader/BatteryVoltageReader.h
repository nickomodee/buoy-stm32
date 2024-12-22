#pragma once

#include "stm32f3xx_hal.h"

class BatteryVoltageReader {
    public:
        BatteryVoltageReader();
        static bool init();
        static float read_voltage();
        static float read_raw_voltage();

    private:
        static GPIO_TypeDef* GPIO_port_;
        static constexpr uint32_t GPIO_pin_ = GPIO_PIN_3;

        static constexpr uint32_t ADC_CHANNEL_ = ADC_CHANNEL_4;
        static constexpr uint8_t ADC_RESOLUTION_ = 12; // 12-bit ADC resolution
        static constexpr float ADC_REFERENCE_VOLTAGE_ = 3.3f;

        static constexpr float VOLTAGE_SCALER_ = 3.0f;

        static ADC_HandleTypeDef hadc1_;

        static bool init_channel_();
        static float ADC_to_voltage_(const uint32_t adc_value);
        static uint32_t read_raw_adc_();
};