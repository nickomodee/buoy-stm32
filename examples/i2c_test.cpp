#include "PAL.h"

#define BME280_ADDRESS 0x76 // or 0x77 for alternate address

// registers
#define BME280_REG_CHIP_ID 0xD0
#define BME280_REG_CTRL_HUM 0xF2
#define BME280_REG_STATUS 0xF3
#define BME280_REG_CTRL_MEAS 0xF4
#define BME280_REG_CONFIG 0xF5
#define BME280_REG_DATA_START 0xF7 // data registers start here

// expected chip ID
#define BME280_CHIP_ID 0x60

// calibration data structure
struct bme280_calib_data {
    // temperature calibration
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;

    // pressure calibration
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;

    // humidity calibration
    uint8_t  dig_H1;
    int16_t  dig_H2;
    uint8_t  dig_H3;
    int16_t  dig_H4;
    int16_t  dig_H5;
    int8_t   dig_H6;
};

// raw sensor data structure
struct raw_data {
    uint32_t pressure;
    uint32_t temperature;
    uint16_t humidity;
};

// global variables
bme280_calib_data calib;
int32_t t_fine; // global t_fine variable

// I2C read/write functions
uint8_t read_register(uint8_t reg) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0;
}

void write_register(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

void burst_read(uint8_t startReg, uint8_t *data, uint8_t length) {
    Wire.beginTransmission(BME280_ADDRESS);
    Wire.write(startReg);
    Wire.endTransmission();
    Wire.requestFrom(BME280_ADDRESS, length);
    for (int i = 0; i < length; i++) {
        if (Wire.available()) {
            data[i] = Wire.read();
        }
    }
}

// sensor configuration functions
void configure_ctrl_meas() {
    // osrs_t = 1 (temperature oversampling x1)
    // osrs_p = 1 (pressure oversampling x1)
    // mode = 3 (normal mode)
    uint8_t ctrl_meas_value = (1 << 5) | (1 << 2) | 0x03;
    write_register(BME280_REG_CTRL_MEAS, ctrl_meas_value);
    Serial.println("Configured ctrl_meas register");
}

void configure_ctrl_hum() {
    // osrs_h = 1 (humidity oversampling x1)
    write_register(BME280_REG_CTRL_HUM, 0x01);
    Serial.println("Configured ctrl_hum register");

    // must rewrite ctrl_meas after changing ctrl_hum
    uint8_t ctrl_meas_value = read_register(BME280_REG_CTRL_MEAS);
    write_register(BME280_REG_CTRL_MEAS, ctrl_meas_value);
    Serial.println("Rewrote ctrl_meas register to apply humidity settings");
}

// calibration data reading
void read_calibration_data() {
    uint8_t calib_data[26];
    burst_read(0x88, calib_data, 26);

    calib.dig_T1 = (uint16_t)((calib_data[1] << 8) | calib_data[0]);
    calib.dig_T2 = (int16_t)((calib_data[3] << 8) | calib_data[2]);
    calib.dig_T3 = (int16_t)((calib_data[5] << 8) | calib_data[4]);

    calib.dig_P1 = (uint16_t)((calib_data[7] << 8) | calib_data[6]);
    calib.dig_P2 = (int16_t)((calib_data[9] << 8) | calib_data[8]);
    calib.dig_P3 = (int16_t)((calib_data[11] << 8) | calib_data[10]);
    calib.dig_P4 = (int16_t)((calib_data[13] << 8) | calib_data[12]);
    calib.dig_P5 = (int16_t)((calib_data[15] << 8) | calib_data[14]);
    calib.dig_P6 = (int16_t)((calib_data[17] << 8) | calib_data[16]);
    calib.dig_P7 = (int16_t)((calib_data[19] << 8) | calib_data[18]);
    calib.dig_P8 = (int16_t)((calib_data[21] << 8) | calib_data[20]);
    calib.dig_P9 = (int16_t)((calib_data[23] << 8) | calib_data[22]);

    calib.dig_H1 = calib_data[25];

    uint8_t calib_data_h[7];
    burst_read(0xE1, calib_data_h, 7);

    calib.dig_H2 = (int16_t)((calib_data_h[1] << 8) | calib_data_h[0]);
    calib.dig_H3 = calib_data_h[2];
    calib.dig_H4 = (int16_t)((calib_data_h[3] << 4) | (calib_data_h[4] & 0x0F));
    calib.dig_H5 = (int16_t)((calib_data_h[5] << 4) | (calib_data_h[4] >> 4));
    calib.dig_H6 = (int8_t)calib_data_h[6];
}

// raw data reading
raw_data read_raw_data() {
    uint8_t data[8];
    burst_read(BME280_REG_DATA_START, data, 8);

    raw_data rd;
    rd.pressure = ((uint32_t)data[0] << 12) | ((uint32_t)data[1] << 4) | ((data[2] >> 4) & 0x0F);
    rd.temperature = ((uint32_t)data[3] << 12) | ((uint32_t)data[4] << 4) | ((data[5] >> 4) & 0x0F);
    rd.humidity = ((uint16_t)data[6] << 8) | data[7];

    return rd;
}

// compensation functions
float compensate_temperature(int32_t adc_T) {
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)calib.dig_T1 << 1))) * ((int32_t)calib.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib.dig_T1)) *
              ((adc_T >> 4) - ((int32_t)calib.dig_T1))) >> 12) *
            ((int32_t)calib.dig_T3)) >> 14;
    t_fine = var1 + var2;
    float T = (t_fine * 5 + 128) >> 8;
    return T / 100.0;
}

float compensate_pressure(int32_t adc_P) {
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib.dig_P3) >> 8) + ((var1 * (int64_t)calib.dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib.dig_P1) >> 33;

    if (var1 == 0) {
        return 0; // Avoid division by zero
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)calib.dig_P8) * p) >> 19;

    p = ((p + var1 + var2) >> 8) + (((int64_t)calib.dig_P7) << 4);
    float pressure = (float)p / 256.0;
    return pressure / 100.0; // Convert to hPa
}

float compensate_humidity(int32_t adc_H) {
    int32_t var1, var2, var3, var4, var5;
    var1 = t_fine - ((int32_t)76800);
    var2 = (int32_t)(adc_H * 16384);
    var3 = (int32_t)(((int32_t)calib.dig_H4) * 1048576);
    var4 = ((int32_t)calib.dig_H5) * var1;
    var5 = (((var2 - var3) - var4) + (int32_t)16384) / 32768;
    var2 = (var1 * ((int32_t)calib.dig_H6)) / 1024;
    var3 = (var1 * ((int32_t)calib.dig_H3)) / 2048;
    var4 = ((var2 * (var3 + (int32_t)32768)) / 1024) + (int32_t)2097152;
    var2 = ((var4 * ((int32_t)calib.dig_H2)) + 8192) / 16384;
    var3 = var5 * var2;
    var4 = ((var3 / 32768) * (var3 / 32768)) / 128;
    var5 = var3 - ((var4 * ((int32_t)calib.dig_H1)) / 16);
    var5 = (var5 < 0 ? 0 : var5);
    var5 = (var5 > 419430400 ? 419430400 : var5);
    uint32_t humidity = (uint32_t)(var5 / 4096);
    return (float)humidity / 1024.0;
}

void setup() {
    Wire.begin();
    Serial.begin(9600);

    Serial.println("BME280 I2C test");

    // check chip ID
    uint8_t chipId = read_register(BME280_REG_CHIP_ID);
    Serial.print("Chip ID: 0x");
    Serial.println(chipId, PAL_HEX);
    if (chipId != BME280_CHIP_ID) {
        Serial.println("BME280 not detected. Please check wiring.");
        while (1); // halt execution
    }

    // sensor initialization
    configure_ctrl_hum();
    configure_ctrl_meas();
    write_register(BME280_REG_CONFIG, 0x00); // optional configuration

    // read calibration data
    read_calibration_data();

    // allow sensor to initialize
    PAL_DELAY(100);
}

void loop() {
    // read raw data
    raw_data rd = read_raw_data();

    // compensate temperature (must be done first to get t_fine)
    float temperature = compensate_temperature(rd.temperature);

    // compensate pressure
    float pressure = compensate_pressure(rd.pressure);

    // compensate humidity
    float humidity = compensate_humidity(rd.humidity);

    // print results
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Pressure: ");
    Serial.print(pressure);
    Serial.println(" hPa");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %RH");

    Serial.println("------------------------");

    // wait before next reading
    PAL_DELAY(2000);
}