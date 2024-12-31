#include "SHT30.h"

SHT30::SHT30(const uint8_t i2c_address/* = SHT30_I2C_ADDRESS*/, PAL_TWOWIRE* wire/* = &PAL_WIRE*/) : i2c_address_(i2c_address), wire_(wire) {}

bool SHT30::init() {
    reset();
    return read_status() != SHT30_STATUS_FAIL;
}

uint16_t SHT30::read_status() {
    write_command_(SHT30_READ_STATUS);
    uint8_t data[3];
    int read_data;
    if (wire_->requestFrom(i2c_address_, ARR_SIZE(data)) != ARR_SIZE(data)) {
        return SHT30_STATUS_FAIL;
    }
    for (uint8_t i = 0; i < ARR_SIZE(data); i++) {
        watchdog.refresh();
        read_data = wire_->read();
        if (read_data == -1) {
            return SHT30_STATUS_FAIL;
        }
        data[i] = (uint8_t)read_data;
    }
    const uint16_t status = ((uint16_t)data[0] << 8) | data[1];
    return status;
}

void SHT30::reset() {
    write_command_(SHT30_SOFT_RESET);
    PAL_DELAY(10);
}

void SHT30::enable_heater() {
    write_command_(SHT30_HEATER_EN);
    PAL_DELAY(1);
}

void SHT30::disable_heater() {
    write_command_(SHT30_HEATER_DIS);
    PAL_DELAY(1);
}

bool SHT30::is_heater_enabled() {
    const uint16_t status = read_status();
    return (status & (1 << SHT30_STATUS_HEATER_BIT)) == 1;
}

float SHT30::read_temp() {
    return read_temp_hum_() ? temp_ : NAN;
}

float SHT30::read_humidity() {
    // first turn off the heater for the initial read, then if it is above the humidity heater threshold we cycle the heater in case of condensation before reading again
    if (is_heater_enabled()) {
        disable_heater();
        PAL_DELAY(SHT30_HEATER_CYCLE_DISABLED_TIME);
    }

    if (!read_temp_hum_()) {
        return NAN;
    }

    if (humidity_ > SHT30_HEATER_HUMIDITY_THRESHOLD) {
        enable_heater();
        PAL_DELAY(SHT30_HEATER_CYCLE_ENABLED_TIME);

        disable_heater();
        PAL_DELAY(SHT30_HEATER_CYCLE_DISABLED_TIME);

        read_temp_hum_();
    }

    return humidity_;
}

// we could do this with hardware but it's so simple and fast to just do it in software
static uint8_t crc8(const uint8_t *data, int len) {
  /*
   *
   * CRC-8 formula from page 14 of SHT spec pdf
   *
   * Test data 0xBE, 0xEF should yield 0x92
   *
   * Initialization data 0xFF
   * Polynomial 0x31 (x8 + x5 +x4 +1)
   * Final XOR 0x00
   */

    const uint8_t POLYNOMIAL(0x31);
    uint8_t crc(0xFF);

    for (int j = len; j; --j) {
        crc ^= *data++;

        for (int i = 8; i; --i) {
            crc = (crc & 0x80) ? (crc << 1) ^ POLYNOMIAL : (crc << 1);
        }
    }
    return crc;
}

bool SHT30::read_temp_hum_() {
    if (!write_command_(SHT30_MEAS_HIGHREP)) {
        return false;
    }

    PAL_DELAY(20);

    uint8_t read_buffer[6];
    int read_data;
    if (wire_->requestFrom(i2c_address_, ARR_SIZE(read_buffer)) != ARR_SIZE(read_buffer)) {
        return false;
    }
    for (uint8_t i = 0; i < ARR_SIZE(read_buffer); i++) {
        watchdog.refresh();
        read_data = wire_->read();
        if (read_data == -1) {
            return false;
        }
        read_buffer[i] = (uint8_t)read_data;
    }

    if ((read_buffer[2] != crc8(read_buffer, 2)) || (read_buffer[5] != crc8(read_buffer + 3, 2))) { // verify checksums
        return false;
    }

    const uint16_t raw_temp = ((uint16_t)read_buffer[0] << 8) | read_buffer[1];
    const int32_t scaled_temp = ((SHT30_TEMP_SCALE_MULTIPLIER * raw_temp) >> SHT30_TEMP_SCALE_SHIFT) - SHT30_TEMP_SCALE_OFFSET;
    temp_ = (float)((float)scaled_temp / SHT30_TEMP_SCALE_DIVISOR);

    const uint16_t raw_humidity = ((uint16_t)read_buffer[3] << 8) | read_buffer[4];
    const int32_t scaled_humidity = (SHT30_HUMIDITY_SCALE_MULTIPLIER * raw_humidity) >> SHT30_HUMIDITY_SCALE_SHIFT;
    humidity_ = (float)((float)scaled_humidity / SHT30_HUMIDITY_SCALE_DIVISOR);

    return true;
}

bool SHT30::write_command_(const uint16_t command) {
    uint8_t command_buffer[2];
    command_buffer[0] = (uint8_t)(command >> 8);
    command_buffer[1] = (uint8_t)(command & 0xFF);
    wire_->beginTransmission(i2c_address_);
    return (wire_->write(command_buffer, ARR_SIZE(command_buffer)) == ARR_SIZE(command_buffer)) && (wire_->endTransmission() == 0);
}