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
    // enable interrupt on every sample
    write_8(SI1145_REG_INTCFG, SI1145_REG_INTCFG_INTOE);
    write_8(SI1145_REG_IRQEN, SI1145_REG_IRQEN_ALSEVERYSAMPLE);

    // measurement rate for auto
    write_8(SI1145_REG_MEASRATE0, 0xFF); // 255 * 31.25uS = 8ms

    // auto run
    write_8(SI1145_REG_COMMAND, SI1145_PSALS_AUTO);

    return true;
}

double SI1145::read_uv_index() {
    return read_16(SI1145_REG_UVINDEX) / 100.0;
}

uint16_t SI1145::read_visible_counts() {
    return read_16(SI1145_REG_VISIBLE);
}

double SI1145::read_visible_lux() {
    return read_visible_counts() / VISIBLE_SUNLIGHT_LUX;
}

uint16_t SI1145::read_ir_counts() {
    return read_16(SI1145_REG_IR);
}

double SI1145::read_ir_lux() {
    return read_ir_counts() / IR_SUNLIGHT_LUX;
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
    return ((uint16_t)wire_->read() << 8) | ((uint16_t)wire_->read());
}

bool SI1145::write_8(const uint8_t reg, const uint8_t data) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    wire_->write(data);
    return wire_->endTransmission() == 0;
}