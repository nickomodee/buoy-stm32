#pragma once

#include "../interfaces/UVSensor.h"
#include "stm32f3xx_hal.h"

class GUVAS12SD : public UVSensor {
    public:
        GUVAS12SD(GPIO_TypeDef* GPIO_port, const uint8_t GPIO_pin, const uint32_t adc_channel);
        bool init() override;
        float read_uv_index() override;
        float read_voltage();

    private:
        bool init_channel_();
        float ADC_to_voltage_(const uint32_t adc_value);
        uint32_t read_raw_adc_();

        GPIO_TypeDef* GPIO_port_;
        const uint8_t GPIO_pin_;
        const uint32_t adc_channel_;

        static constexpr uint8_t ADC_RESOLUTION_ = 12; // 12-bit ADC resolution
        static constexpr float ADC_REFERENCE_VOLTAGE_ = 3.3f;

        static ADC_HandleTypeDef hadc_;
};