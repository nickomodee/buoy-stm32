#include "PAL.h"
#include "DS18B20.h"
#include "BME280.h"
#include "SHT30.h"
#include "TempSensor.h"
#include "PressureSensor.h"
#include "HumiditySensor.h"

#define ONEWIRE_PIN_AIR 3
#define ONEWIRE_PIN_WATER 9

DS18B20 ds18b20_air(ONEWIRE_PIN_AIR);
DS18B20 ds18b20_water(ONEWIRE_PIN_WATER);
TempSensor& temp_sensor_ds18b20_air = ds18b20_air;
TempSensor& temp_sensor_ds18b20_water = ds18b20_water;

BME280 bme280;
TempSensor& temp_sensor_bme280 = bme280;
PressureSensor& pressure_sensor_bme280 = bme280;
HumiditySensor& humidity_sensor_bme280 = bme280;

SHT30 sht30;
TempSensor& temp_sensor_sht30 = sht30;
HumiditySensor& humidity_sensor_sht30 = sht30;

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();
    while (!temp_sensor_ds18b20_air.init()) {
        PAL_SERIAL.println("DS18B20 air initialisation failed");
        PAL_DELAY(1000);
    }
    while (!temp_sensor_ds18b20_water.init()) {
        PAL_SERIAL.println("DS18B20 water initialisation failed");
        PAL_DELAY(1000);
    }
    while (!bme280.init()) {
        PAL_SERIAL.println("BME280 initialisation failed");
        PAL_DELAY(1000);
    }
    while (!sht30.init()) {
        PAL_SERIAL.println("SHT30 initialisation failed");
        PAL_DELAY(1000);
    }
}

void loop() {
    const float temperature_ds18b20_air = temp_sensor_ds18b20_air.read_temp();

    PAL_SERIAL.print("Temperature DS18B20 air: ");
    PAL_SERIAL.print(temperature_ds18b20_air, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");


    const float temperature_ds18b20_water = temp_sensor_ds18b20_water.read_temp();

    PAL_SERIAL.print("Temperature DS18B20 water: ");
    PAL_SERIAL.print(temperature_ds18b20_water, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    const float temperature_bme280 = temp_sensor_bme280.read_temp();
    const float pressure_bme280 = pressure_sensor_bme280.read_pressure();
    const float humidity_bme280 = humidity_sensor_bme280.read_humidity();


    PAL_SERIAL.print("Temperature BME280: ");
    PAL_SERIAL.print(temperature_bme280, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    PAL_SERIAL.print("Pressure BME280: ");
    PAL_SERIAL.print(pressure_bme280 / 100.0, 2); // convert Pa to hPa and print with 2 d.p.
    PAL_SERIAL.println(" hPa");

    PAL_SERIAL.print("Humidity BME280: ");
    PAL_SERIAL.print(humidity_bme280, 2); // 2 d.p.
    PAL_SERIAL.println("%");


    const float temperature_sht30 = temp_sensor_sht30.read_temp();
    const float humidity_sht30 = humidity_sensor_sht30.read_humidity();

    PAL_SERIAL.print("Temperature SHT30: ");
    PAL_SERIAL.print(temperature_sht30, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    PAL_SERIAL.print("Humidity SHT30: ");
    PAL_SERIAL.print(humidity_sht30, 2); // 2 d.p.
    PAL_SERIAL.println("%");


    PAL_SERIAL.println();
    PAL_DELAY(2000);
}