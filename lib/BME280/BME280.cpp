#include "BME280.h"

Config BME280::config_reg;
CtrlMeas BME280::meas_reg;
CtrlHum BME280::hum_reg;

BME280::BME280(const uint8_t i2c_address, PAL_TWOWIRE* wire) : i2c_address_(i2c_address), wire_(wire) {};

bool BME280::init() {
    if (read_8(BME280_REGISTER_CHIPID) != BME280_CHIP_ID) {
        return false; // not a valid BME280 chip
    }

    write_8(BME280_REGISTER_SOFTRESET, BME280_SOFTRESET_VALUE);
    
    PAL_DELAY(10);

    while (is_reading_calibration()) {
        PAL_DELAY(10);
    }

    read_coefficients();

    set_sampling(); // use defaults

    PAL_DELAY(100);

    return true;
}

// Modified from: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.cpp
void BME280::set_sampling(SensorMode mode, SensorSampling temp_sampling, SensorSampling press_sampling, SensorSampling hum_sampling, SensorFilter filter, StandbyDuration duration) {
    BME280::meas_reg.mode = (uint8_t)mode;
    BME280::meas_reg.osrs_t = (uint8_t)temp_sampling;
    BME280::meas_reg.osrs_p = (uint8_t)press_sampling;

    BME280::hum_reg.osrs_h = (uint8_t)hum_sampling;
    BME280::config_reg.filter = (uint8_t)filter;
    BME280::config_reg.t_sb = (uint8_t)duration;
    BME280::config_reg.spi3w_en = 0;

    // making sure sensor is in sleep mode before setting configuration as it otherwise may be ignored
    write_8(BME280_REGISTER_CONTROL, (uint8_t)SensorMode::MODE_SLEEP);

    // you must make sure to also set REGISTER_CONTROL after setting the CONTROLHUMID register, otherwise the values won't be applied (see DS 5.4.3)
    write_8(BME280_REGISTER_CONTROLHUMID, BME280::hum_reg.get());
    write_8(BME280_REGISTER_CONFIG, BME280::config_reg.get());
    write_8(BME280_REGISTER_CONTROL, BME280::meas_reg.get());
}

void BME280::read_coefficients() {
    // temperature coefficients
    status_.dig_T1 = read_16_LE(BME280_REGISTER_DIG_T1);
    status_.dig_T2 = (int16_t)read_16_LE(BME280_REGISTER_DIG_T2);
    status_.dig_T3 = (int16_t)read_16_LE(BME280_REGISTER_DIG_T3);

    // pressure coefficients
    status_.dig_P1 = read_16_LE(BME280_REGISTER_DIG_P1);
    status_.dig_P2 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P2);
    status_.dig_P3 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P3);
    status_.dig_P4 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P4);
    status_.dig_P5 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P5);
    status_.dig_P6 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P6);
    status_.dig_P7 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P7);
    status_.dig_P8 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P8);
    status_.dig_P9 = (int16_t)read_16_LE(BME280_REGISTER_DIG_P9);

    // humidity coefficients
    status_.dig_H1 = read_8(BME280_REGISTER_DIG_H1);
    status_.dig_H2 = (int16_t)read_16_LE(BME280_REGISTER_DIG_H2);
    status_.dig_H3 = read_8(BME280_REGISTER_DIG_H3);
    status_.dig_H4 = ((int8_t)read_8(BME280_REGISTER_DIG_H4) << 4) | (read_8(BME280_REGISTER_DIG_H4 + 1) & 0xF);
    status_.dig_H5 = ((int8_t)read_8(BME280_REGISTER_DIG_H5 + 1) << 4) | (read_8(BME280_REGISTER_DIG_H5) >> 4);
    status_.dig_H6 = (int8_t)read_8(BME280_REGISTER_DIG_H6);
}

bool BME280::is_reading_calibration() {
    return (read_8(BME280_REGISTER_STATUS) & 1) != 0;
}

// Modifed from: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.cpp, cause WTF is happening
int32_t BME280::read_temp_fine() {
    int32_t var1, var2;

    int32_t adc_T = read_24(BME280_REGISTER_TEMPDATA);
    if (adc_T == 0x800000) { // value in case temp measurement was disabled
        return -1;
    }
    adc_T >>= 4;

    var1 = (int32_t)((adc_T / 8) - ((int32_t)status_.dig_T1 * 2));
    var1 = (var1 * ((int32_t)status_.dig_T2)) / 2048;
    var2 = (int32_t)((adc_T / 16) - ((int32_t)status_.dig_T1));
    var2 = (((var2 * var2) / 4096) * ((int32_t)status_.dig_T3)) / 16384;

    return var1 + var2; // + t_fine_adjust;
}

// Modifed from: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.cpp, cause WTF is happening
double BME280::read_temp() {
    const int32_t t_fine = read_temp_fine();
    if (t_fine == -1) { // temp measurement was disabled
        return NAN;
    }

    return (double)((t_fine * 5 + 128) / 256) / 100;
}

// Modifed from: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.cpp, cause WTF is happening
double BME280::read_pressure() {
    int64_t var1, var2, var3, var4;

    const int32_t t_fine = read_temp_fine();
    if (t_fine == -1) { // temp measurement was disabled
        return NAN;
    }

    int32_t adc_P = read_24(BME280_REGISTER_PRESSUREDATA);
    if (adc_P == 0x800000) { // value in case pressure measurement was disabled
        return NAN;
    }
    adc_P >>= 4;

    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)status_.dig_P6;
    var2 = var2 + ((var1 * (int64_t)status_.dig_P5) * 131072);
    var2 = var2 + (((int64_t)status_.dig_P4) * 34359738368);
    var1 = ((var1 * var1 * (int64_t)status_.dig_P3) / 256) + ((var1 * ((int64_t)status_.dig_P2) * 4096));
    var3 = ((int64_t)1) * 140737488355328;
    var1 = (var3 + var1) * ((int64_t)status_.dig_P1) / 8589934592;

    if (var1 == 0) {
        return 0; // avoid exception caused by division by zero
    }

    var4 = 1048576 - adc_P;
    var4 = (((var4 * 2147483648) - var2) * 3125) / var1;
    var1 = (((int64_t)status_.dig_P9) * (var4 / 8192) * (var4 / 8192)) / 33554432;
    var2 = (((int64_t)status_.dig_P8) * var4) / 524288;
    var4 = ((var4 + var1 + var2) / 256) + (((int64_t)status_.dig_P7) * 16);

    const float P = var4 / 256.0;

    return P;
}

// Modifed from: https://github.com/adafruit/Adafruit_BME280_Library/blob/master/Adafruit_BME280.cpp, cause WTF is happening
double BME280::read_humidity() {
    int32_t var1, var2, var3, var4, var5;

    const int32_t t_fine = read_temp_fine();
    if (t_fine == -1) { // temp measurement was disabled
        return NAN;
    }

    int32_t adc_H = read_16(BME280_REGISTER_HUMIDDATA);
    if (adc_H == 0x8000) { // value in case humidity measurement was disabled
        return NAN;
    }

    var1 = t_fine - ((int32_t)76800);
    var2 = (int32_t)(adc_H * 16384);
    var3 = (int32_t)(((int32_t)status_.dig_H4) * 1048576);
    var4 = ((int32_t)status_.dig_H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)status_.dig_H6)) / 1024;
    var3 = (var1 * ((int32_t)status_.dig_H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)status_.dig_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)status_.dig_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    const uint32_t H = (uint32_t)(var5 / 4096);

    return (float)H / 1024.0;
}

uint8_t BME280::read_8(const uint8_t reg) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission() != 0) { // error
        return -1; // idk what to do here
    }
    wire_->requestFrom(i2c_address_, 1);
    return wire_->read();
}

uint16_t BME280::read_16(const uint8_t reg) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission() != 0) { // error
        return -1; // idk what to do here
    }
    wire_->requestFrom(i2c_address_, 2);
    return ((uint16_t)wire_->read() << 8) | ((uint16_t)wire_->read());
}

uint16_t BME280::read_16_LE(const uint8_t reg) {
    const uint16_t data = read_16(reg);
    return (data >> 8) | (data << 8);
}

uint32_t BME280::read_24(const uint8_t reg) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    if (wire_->endTransmission() != 0) { // error
        return -1; // idk what to do here
    }
    wire_->requestFrom(i2c_address_, 3);
    return ((uint32_t)wire_->read() << 16) | ((uint32_t)wire_->read() << 8) | ((uint32_t)wire_->read());
}

bool BME280::write_8(const uint8_t reg, const uint8_t data) {
    wire_->beginTransmission(i2c_address_);
    wire_->write(reg);
    wire_->write(data);
    return wire_->endTransmission() == 0;
}