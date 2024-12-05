#pragma once

#include "../interfaces/UVSensor.h"
#include "../interfaces/VisibleLuxSensor.h"
#include "../interfaces/IRLuxSensor.h"
#include "../PAL/PAL.h"

// page 6/7 of the SI1145 datasheet: https://www.silabs.com/documents/public/data-sheets/Si1145-46-47.pdf
// visible photodiode response (ALS_VIS)
#define VISIBLE_SUNLIGHT_LUX (0.282) // ADC counts/lux for Sunlight
#define VISIBLE_INCANDESCENT_LUX (0.319) // ADC counts/lux for 2500K Incandescent bulb
#define VISIBLE_FLUORESCENT_LUX (0.146) // ADC counts/lux for "Cool white" Fluorescent
#define VISIBLE_IR_LED_LUX (8.277) // ADC counts·m^2/W for Infrared LED (875 nm)
// infrared photodiode response (ALS_IR)
#define IR_SUNLIGHT_LUX (2.44) // ADC counts/lux for Sunlight
#define IR_INCANDESCENT_LUX (8.46) // ADC counts/lux for 2500K Incandescent bulb
#define IR_FLUORESCENT_LUX (0.71) // ADC counts/lux for "Cool white" Fluorescent
#define IR_IR_LED_LUX (452.38) // ADC counts·m^2/W for Infrared LED (875 nm)

#define SI1145_I2C_ADDRESS 0x60 // default address

// values
#define SI1145_PART_ID (0x45)
#define SI1145_RESET (0x01)
#define SI1145_PSALS_AUTO (0x0F)

// parameters
#define SI1145_PARAM_SET (0xA0)
#define SI1145_PARAM_CHLIST (0x01)
#define SI1145_PARAM_CHLIST_ENUV (0x80)
#define SI1145_PARAM_CHLIST_ENALSIR (0x20)
#define SI1145_PARAM_CHLIST_ENALSVIS (0x10)
#define SI1145_PARAM_CHLIST_ENPS1 (0x01)

// registers
#define SI1145_REG_PARTID (0x00)
#define SI1145_REG_MEASRATE0 (0x08)
#define SI1145_REG_MEASRATE1 (0x09)
#define SI1145_REG_IRQEN (0x04)
#define SI1145_REG_IRQMODE1 (0x05)
#define SI1145_REG_IRQMODE2 (0x06)
#define SI1145_REG_INTCFG (0x03)
#define SI1145_REG_IRQSTAT (0x21)
#define SI1145_REG_COMMAND (0x18)
#define SI1145_REG_HWKEY (0x07)
#define SI1145_REG_UCOEFF0 (0x13)
#define SI1145_REG_UCOEFF1 (0x14)
#define SI1145_REG_UCOEFF2 (0x15)
#define SI1145_REG_UCOEFF3 (0x16)
#define SI1145_REG_PARAMWR (0x17)
#define SI1145_REG_PARAMRD (0x2E)
#define SI1145_REG_INTCFG_INTOE (0x01)
#define SI1145_REG_IRQEN_ALSEVERYSAMPLE (0x01)
#define SI1145_REG_UVINDEX (0x2C)
#define SI1145_REG_VISIBLE (0x22)
#define SI1145_REG_IR (0x24)

// Modified from: https://github.com/adafruit/Adafruit_SI1145_Library/blob/master/Adafruit_SI1145.h and https://github.com/adafruit/Adafruit_SI1145_Library/blob/master/Adafruit_SI1145.cpp
class SI1145 : public UVSensor, public VisibleLuxSensor, public IRLuxSensor {
    public:
        SI1145(const uint8_t i2c_address = SI1145_I2C_ADDRESS, PAL_TWOWIRE* wire = &PAL_WIRE);
        bool init() override;
        void reset();
        double read_uv_index() override;
        double read_visible_lux() override;
        double read_ir_lux() override;
    private:
        uint16_t read_visible_counts(); // the raw ADC counts which need to be converted to lux
        uint16_t read_ir_counts(); // the raw ADC counts which need to be converted to lux
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

        // write methods
        uint8_t write_param(const uint8_t param, const uint8_t value);
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
};