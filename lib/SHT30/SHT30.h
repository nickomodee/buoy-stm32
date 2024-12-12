#pragma once

#include "../PAL/PAL.h"
#include "../interfaces/TempSensor.h"
#include "../interfaces/HumiditySensor.h"
#include <cstdint>

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

// values
#define SHT30_I2C_ADDRESS 0x44
#define SHT30_STATUS_FAIL 0xFFFF
#define SHT30_STATUS_HEATER_BIT 0x0D /**< Status Register Heater Bit */
#define SHT30_HEATER_HUMIDITY_THRESHOLD 0.90 ///< the threshold humidity RH% to cycle the heater in case of condensation
#define SHT30_HEATER_CYCLE_ENABLED_TIME 1000 ///< the time to enable the heater to prevent condensation
#define SHT30_HEATER_CYCLE_DISABLED_TIME 1000 ///< the time to enable the heater to prevent condensation
#define SHT30_TEMP_SCALE_MULTIPLIER 4375
#define SHT30_TEMP_SCALE_SHIFT 14
#define SHT30_TEMP_SCALE_OFFSET 4500
#define SHT30_TEMP_SCALE_DIVISOR 100.0f
#define SHT30_HUMIDITY_SCALE_MULTIPLIER 625
#define SHT30_HUMIDITY_SCALE_SHIFT 12
#define SHT30_HUMIDITY_SCALE_DIVISOR 100.0f

// commands
#define SHT30_MEAS_HIGHREP_STRETCH 0x2C06 /**< Measurement High Repeatability with Clock Stretch Enabled */
#define SHT30_MEAS_MEDREP_STRETCH 0x2C0D /**< Measurement Medium Repeatability with Clock Stretch Enabled */
#define SHT30_MEAS_LOWREP_STRETCH 0x2C10 /**< Measurement Low Repeatability with Clock Stretch Enabled*/
#define SHT30_MEAS_HIGHREP 0x2400 /**< Measurement High Repeatability with Clock Stretch Disabled */
#define SHT30_MEAS_MEDREP 0x240B /**< Measurement Medium Repeatability with Clock Stretch Disabled */
#define SHT30_MEAS_LOWREP 0x2416 /**< Measurement Low Repeatability with Clock Stretch Disabled */
#define SHT30_READ_STATUS 0xF32D /**< Read Out of Status Register */
#define SHT30_CLEAR_STATUS 0x3041 /**< Clear Status */
#define SHT30_SOFT_RESET 0x30A2 /**< Soft Reset */
#define SHT30_HEATER_EN 0x306D /**< Heater Enable */
#define SHT30_HEATER_DIS 0x3066 /**< Heater Disable */

// modified from: https://github.com/adafruit/Adafruit_SHT31/blob/master/Adafruit_SHT31.h and https://github.com/adafruit/Adafruit_SHT31/blob/master/Adafruit_SHT31.cpp
class SHT30 : public TempSensor, public HumiditySensor {
    public:
        SHT30(const uint8_t i2c_address = SHT30_I2C_ADDRESS, PAL_TWOWIRE* wire = &PAL_WIRE);
        bool init() override;
        float read_temp() override;
        float read_humidity() override;
        uint16_t read_status();
        void reset();
        void enable_heater();
        void disable_heater();
        bool is_heater_enabled();

    private:
        bool read_temp_hum_();
        bool write_command_(const uint16_t command);

        float temp_;
        float humidity_;
        const uint8_t i2c_address_;
        PAL_TWOWIRE* wire_;
};