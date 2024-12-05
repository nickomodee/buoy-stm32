#pragma once

#include "../interfaces/TempSensor.h"
#include "../interfaces/PressureSensor.h"
#include "../interfaces/HumiditySensor.h"
#include "../PAL/PAL.h"

#define BME280_PRIMARY_I2C_ADDRESS (0x76)

// values
#define BME280_CHIP_ID (0x60) // expected chip id
#define BME280_SOFTRESET_VALUE (0xB6)

// registers
#define BME280_REGISTER_CHIPID (0xD0)
#define BME280_REGISTER_SOFTRESET (0xE0)
#define BME280_REGISTER_STATUS (0xF3)
#define BME280_REGISTER_DIG_T1 (0x88)
#define BME280_REGISTER_DIG_T2 (0x8A)
#define BME280_REGISTER_DIG_T3 (0x8C)
#define BME280_REGISTER_DIG_P1 (0x8E)
#define BME280_REGISTER_DIG_P2 (0x90)
#define BME280_REGISTER_DIG_P3 (0x92)
#define BME280_REGISTER_DIG_P4 (0x94)
#define BME280_REGISTER_DIG_P5 (0x96)
#define BME280_REGISTER_DIG_P6 (0x98)
#define BME280_REGISTER_DIG_P7 (0x9A)
#define BME280_REGISTER_DIG_P8 (0x9C)
#define BME280_REGISTER_DIG_P9 (0x9E)
#define BME280_REGISTER_DIG_H1 (0xA1)
#define BME280_REGISTER_DIG_H2 (0xE1)
#define BME280_REGISTER_DIG_H3 (0xE3)
#define BME280_REGISTER_DIG_H4 (0xE4)
#define BME280_REGISTER_DIG_H5 (0xE5)
#define BME280_REGISTER_DIG_H6 (0xE7)
#define BME280_REGISTER_TEMPDATA (0xFA)
#define BME280_REGISTER_PRESSUREDATA (0xF7)
#define BME280_REGISTER_HUMIDDATA (0xFD)
#define BME280_REGISTER_CONTROLHUMID (0xF2)
#define BME280_REGISTER_CONTROL (0xF4)
#define BME280_REGISTER_CONFIG (0xF5)

namespace {
    struct BME280Status {
        // temperature compensation values
        uint16_t dig_T1;
        int16_t dig_T2;
        int16_t dig_T3;

        // pressure compensation values
        uint16_t dig_P1;
        int16_t dig_P2;
        int16_t dig_P3;
        int16_t dig_P4;
        int16_t dig_P5;
        int16_t dig_P6;
        int16_t dig_P7;
        int16_t dig_P8;
        int16_t dig_P9;

        // humidity compensation values
        uint8_t dig_H1;
        int16_t dig_H2;
        uint8_t dig_H3;
        int16_t dig_H4;
        int16_t dig_H5;
        int8_t dig_H6;
    };

    struct Config {
        // inactive duration (standby time) in normal mode
        // 000 = 0.5 ms
        // 001 = 62.5 ms
        // 010 = 125 ms
        // 011 = 250 ms
        // 100 = 500 ms
        // 101 = 1000 ms
        // 110 = 10 ms
        // 111 = 20 ms
        uint8_t t_sb : 3; ///< inactive duration (standby time) in normal mode

        // filter settings
        // 000 = filter off
        // 001 = 2x filter
        // 010 = 4x filter
        // 011 = 8x filter
        // 100 and above = 16x filter
        uint8_t filter : 3; ///< filter settings

        // unused - don't set
        uint8_t none : 1;     ///< unused - don't set
        uint8_t spi3w_en : 1; ///< unused - don't set

        /// @returns combined config register
        uint8_t get() { return (t_sb << 5) | (filter << 2) | spi3w_en; }
    };

    struct CtrlMeas {
        // temperature oversampling
        // 000 = skipped
        // 001 = x1
        // 010 = x2
        // 011 = x4
        // 100 = x8
        // 101 and above = x16
        uint8_t osrs_t : 3; ///< temperature oversampling

        // pressure oversampling
        // 000 = skipped
        // 001 = x1
        // 010 = x2
        // 011 = x4
        // 100 = x8
        // 101 and above = x16
        uint8_t osrs_p : 3; ///< pressure oversampling

        // device mode
        // 00       = sleep
        // 01 or 10 = forced
        // 11       = normal
        uint8_t mode : 2; ///< device mode

        /// @returns combined ctrl register
        uint8_t get() { return (osrs_t << 5) | (osrs_p << 2) | mode; }
    };

    struct CtrlHum {
        /// unused - don't set
        uint8_t none : 5;

        // pressure oversampling
        // 000 = skipped
        // 001 = x1
        // 010 = x2
        // 011 = x4
        // 100 = x8
        // 101 and above = x16
        uint8_t osrs_h : 3; ///< pressure oversampling

        /// @returns combined ctrl hum register
        uint8_t get() { return osrs_h; }
    };
}

/**************************************************************************/
/*!
    @brief  sampling rates
*/
/**************************************************************************/
enum class SensorSampling {
    SAMPLING_NONE = 0b000,
    SAMPLING_X1 = 0b001,
    SAMPLING_X2 = 0b010,
    SAMPLING_X4 = 0b011,
    SAMPLING_X8 = 0b100,
    SAMPLING_X16 = 0b101
};

/**************************************************************************/
/*!
    @brief  power modes
*/
/**************************************************************************/
enum class SensorMode {
    MODE_SLEEP = 0b00,
    MODE_FORCED = 0b01,
    MODE_NORMAL = 0b11
};

/**************************************************************************/
/*!
    @brief  filter values
*/
/**************************************************************************/
enum class SensorFilter {
    FILTER_OFF = 0b000,
    FILTER_X2 = 0b001,
    FILTER_X4 = 0b010,
    FILTER_X8 = 0b011,
    FILTER_X16 = 0b100
};

/**************************************************************************/
/*!
    @brief  standby duration in ms
*/
/**************************************************************************/
enum class StandbyDuration {
    STANDBY_MS_0_5 = 0b000,
    STANDBY_MS_10 = 0b110,
    STANDBY_MS_20 = 0b111,
    STANDBY_MS_62_5 = 0b001,
    STANDBY_MS_125 = 0b010,
    STANDBY_MS_250 = 0b011,
    STANDBY_MS_500 = 0b100,
    STANDBY_MS_1000 = 0b101
};

// Modified from: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.h and https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.cpp
class BME280 : public TempSensor, public PressureSensor, public HumiditySensor {
    public:
        BME280(const uint8_t i2c_address = BME280_PRIMARY_I2C_ADDRESS, PAL_TWOWIRE* wire = &PAL_WIRE);
        bool init() override;
        void set_sampling(SensorMode mode = SensorMode::MODE_NORMAL,
                          SensorSampling temp_sampling = SensorSampling::SAMPLING_X16,
                          SensorSampling press_sampling = SensorSampling::SAMPLING_X16,
                          SensorSampling hum_sampling = SensorSampling::SAMPLING_X16,
                          SensorFilter filter = SensorFilter::FILTER_OFF,
                          StandbyDuration duration = StandbyDuration::STANDBY_MS_0_5);
        double read_temp() override;
        double read_pressure() override;
        double read_humidity() override;
    private:
        /**
         * @brief Checks the status of whether the device is busy reading calibration data.
         * 
         * @returns A boolean if the device is busy.
         */
        bool is_reading_calibration();
        /**
         * @brief Reads the factory-set coefficients.
         * Saves the values in internal `status_`.
         */
        void read_coefficients();
        int32_t read_temp_fine(); // a high resolution temperature value used for the BME280 pressure and humidity calculations
        // read methods
        /**
         * @brief Read 8 bits from the device on I2C.
         * 
         * @param[in] reg The I2C device register to read from.
         * @returns The uint8_t value at that register with the 8 bits.
         */
        uint8_t read_8(const uint8_t reg);
        /**
         * @brief Read 16 bits from the device on I2C in big endian format.
         * 
         * @param[in] reg The I2C device register to read from.
         * @returns The uint16_t value at that register with the 16 bits in big endian format.
         */
        uint16_t read_16(const uint8_t reg);
        /**
         * @brief Read 16 bits from the device on I2C in little endian format.
         * 
         * @param[in] reg The I2C device register to read from.
         * @returns The uint16_t value at that register with the 16 bits in little endian format.
         */
        uint16_t read_16_LE(const uint8_t reg);
        /**
         * @brief Read 24 bits from the device on I2C in big endian format.
         * 
         * @param[in] reg The I2C device register to read from.
         * @returns The uint32_t value at that register with the 24 bits in big endian format.
         */
        uint32_t read_24(const uint8_t reg);

        // write methods
        /**
         * @brief Write 8 bits to the device on I2C.
         * 
         * @param[in] reg The I2C device register to write to.
         * @param[in] data The data to write to the device register.
         * @returns A boolean if the write was successful
         */
        bool write_8(const uint8_t reg, const uint8_t data);

        const uint8_t i2c_address_;
        PAL_TWOWIRE* wire_;
        BME280Status status_;
        Config config_reg_;
        CtrlMeas meas_reg_;
        CtrlHum hum_reg_;
};