#include "SI1145.h"

SI1145::SI1145(const uint8_t i2c_address, PAL_TWOWIRE* wire) : i2c_address_(i2c_address), wire_(wire) {};

bool SI1145::init() {
    uint8_t id = read_8(SI1145_REG_PARTID);
    if (id != SI1145_PART_ID) { // look for SI1145
        return false;
    }

    reset();

    // enable UV index measurement coefficients
    write_8(SI1145_REG_UCOEFF0, 0x29);
    write_8(SI1145_REG_UCOEFF1, 0x89);
    write_8(SI1145_REG_UCOEFF2, 0x02);
    write_8(SI1145_REG_UCOEFF3, 0x00);

    // enable UV sensor
    write_param(SI1145_PARAM_CHLIST, SI1145_PARAM_CHLIST_ENUV | SI1145_PARAM_CHLIST_ENALSIR | SI1145_PARAM_CHLIST_ENALSVIS | SI1145_PARAM_CHLIST_ENPS1);

    // write_8(SI1145_REG_PS_LED21, SI1145_PS21_DISABLED);
    // write_8(SI1145_REG_PS_LED3, SI1145_PS3_DISABLED);

    write_param(SI1145_ALS_VIS_ADC_GAIN, SI1145_ADC_GAIN_DIV1);
    write_param(SI1145_ALS_VIS_ADC_COUNTER, SI1145_ADC_COUNTER_511ADCCLK);
    write_param(SI1145_ALS_VIS_ADC_MISC, SI1145_ADC_MISC_LOWRANGE);

    write_param(SI1145_ALS_IR_ADC_GAIN, SI1145_ADC_GAIN_DIV1);
    write_param(SI1145_ALS_IR_ADC_COUNTER, SI1145_ADC_COUNTER_511ADCCLK);
    write_param(SI1145_ALS_IR_ADC_MISC, SI1145_ADC_MISC_LOWRANGE);

    // // enable interrupt on every sample
    // write_8(SI1145_REG_INTCFG, SI1145_REG_INTCFG_INTOE);
    // write_8(SI1145_REG_IRQEN, SI1145_REG_IRQEN_ALSEVERYSAMPLE);
    // disable interrupts
    write_8(SI1145_REG_INTCFG, SI1145_REG_INTCFG_DISABLED);
    write_8(SI1145_REG_IRQEN, SI1145_REG_IRQEN_DISABLED);

    // measurement rate for auto
    write_8(SI1145_REG_MEASRATE0, 0xFF); // 255 * 31.25uS = 8ms
    // // forced measurement rate
    // write_8(SI1145_REG_MEASRATE0, SI1145_FORCED_MEASRATE0);
    // write_8(SI1145_REG_MEASRATE1, SI1145_FORCED_MEASRATE1);

    write_8(SI1145_REG_COMMAND, SI1145_PSALS_AUTO); // auto run mode
    // write_8(SI1145_REG_COMMAND, SI1145_PSALS_FORCE); // forced conversion mode

    return true;
}

float SI1145::read_uv_index() {
    return read_16(SI1145_REG_UVINDEX) / 100.0f;
}

uint16_t SI1145::read_visible() {
    return read_16(SI1145_REG_VISIBLE);
}

uint16_t SI1145::read_ir() {
    return read_16(SI1145_REG_IR);
}

// source: https://github.com/wollewald/SI1145_WE/blob/master/examples/SI1145_lux_calculation/SI1145_lux_calculation.ino
float SI1145::calculate_lux(const uint16_t visible, const uint16_t ir) {
    const uint16_t visible_dark = 258; // this is my empirical value
    const uint16_t ir_dark = 250; // this is my empirical value

    const float gain_factor = 1.0f;
    const float visible_coeff = 5.41f; // application notes AN523
    const float ir_coeff = 0.08f; // application notes AN523

    // const float visible_count_per_lux = VISIBLE_FLUORESCENT_LUX; // change for light source
    // const float ir_count_per_lux = IR_FLUORESCENT_LUX; // change for light source
    // const float corr_factor = 0.18; // his empirical correction factor

    // according to application notes AN523: 
    const float lux = ((visible - visible_dark) * visible_coeff - (ir - ir_dark) * ir_coeff) * gain_factor;

    // // the equation above does not consider the counts/Lux depending on light source type
    // // he suggests the following equation
    // const float lux = (((visible - visible_dark) / visible_count_per_lux) * visible_coeff - ((ir - ir_dark) / ir_count_per_lux) * ir_coeff) * gain_factor * corr_factor;

    return lux;
}

void SI1145::reset() {
    write_8(SI1145_REG_MEASRATE0, 0);
    write_8(SI1145_REG_MEASRATE1, 0);
    write_8(SI1145_REG_IRQEN, 0);
    write_8(SI1145_REG_IRQMODE1, 0);
    write_8(SI1145_REG_IRQMODE2, 0);
    write_8(SI1145_REG_INTCFG, 0);
    write_8(SI1145_REG_IRQSTAT, 0xFF);

    write_8(SI1145_REG_COMMAND, SI1145_RESET);
    PAL_DELAY(10);
    write_8(SI1145_REG_HWKEY, 0x17);

    PAL_DELAY(10);
}

uint8_t SI1145::write_param(const uint8_t param, const uint8_t value) {
    write_8(SI1145_REG_PARAMWR, value);
    write_8(SI1145_REG_COMMAND, param | SI1145_PARAM_SET);
    return read_8(SI1145_REG_PARAMRD);
}

uint8_t SI1145::read_8(const uint8_t reg) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission() != 0) { // error
        return -1; // idk what to do here
    }
    wire_->requestFrom(i2c_address_, 1);
    return wire_->read();
}

uint16_t SI1145::read_16(const uint8_t reg) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission() != 0) { // error
        return -1; // idk what to do here
    }
    wire_->requestFrom(i2c_address_, 2);
    return ((uint16_t)wire_->read()) | ((uint16_t)wire_->read() << 8); // little endian format
}

bool SI1145::write_8(const uint8_t reg, const uint8_t data) {
    wire_->beginTransmission(i2c_address_);
    const uint8_t buffer[2] = {reg, data};
    wire_->write(buffer, ARR_SIZE(buffer));
    return wire_->endTransmission() == 0;
}