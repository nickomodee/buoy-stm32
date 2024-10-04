#include "PAL.h"

#define I2C_ADDRESS 0x76 // or 0x77 for alternate address
#define CHIP_ID_REGISTER 0xD0 // chip id register to distinguish between BME280 (0x60) and BMP280 (0x58)
#define BME280_CHIP_ID 0x60
#define BMP280_CHIP_ID 0x58

void setup() {
    Wire.begin();
    PAL_SERIAL.begin(9600);

    Wire.beginTransmission(I2C_ADDRESS);
    Wire.write(CHIP_ID_REGISTER);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDRESS, 1);
    if (Wire.available()) {
        const uint8_t chip_id = Wire.read();
        PAL_SERIAL.print("Chip ID: 0x");
        PAL_SERIAL.println(chip_id, PAL_HEX);

        if (chip_id == BME280_CHIP_ID) {
            PAL_SERIAL.println("BME280 detected");
        } else if (chip_id == BMP280_CHIP_ID) {
            PAL_SERIAL.println("BMP280 detected");
        } else {
            PAL_SERIAL.println("Unknown sensor");
        }
    }
}

void loop() {}