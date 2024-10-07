#include "PAL.h"
#include "DS18B20.h"
#include "BME280.h"
#include "TempSensor.h"
#include "PressureSensor.h"
#include "HumiditySensor.h"

#define ONEWIRE_PIN 7

DS18B20 ds18b20(ONEWIRE_PIN);
TempSensor& temp_sensor_ds18b20 = ds18b20;

BME280 bme280;
TempSensor& temp_sensor_bme280 = bme280;
PressureSensor& pressure_sensor = bme280;
HumiditySensor& humidity_sensor = bme280;

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();
    while (!ds18b20.init()) {
        PAL_SERIAL.println("DS18B20 initialisation failed");
        PAL_DELAY(1000);
    }
    while (!bme280.init()) {
        PAL_SERIAL.println("BME280 initialisation failed");
        PAL_DELAY(1000);
    }
}

void loop() {
    const float temperature_ds18b20 = temp_sensor_ds18b20.read_temp();

    PAL_SERIAL.print("Temperature DS18B20: ");
    PAL_SERIAL.print(temperature_ds18b20, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    const float temperature_bme280 = temp_sensor_bme280.read_temp();
    const float pressure = pressure_sensor.read_pressure();
    const float humidity = humidity_sensor.read_humidity();

    PAL_SERIAL.print("Temperature BME280: ");
    PAL_SERIAL.print(temperature_bme280, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    PAL_SERIAL.print("Pressure: ");
    PAL_SERIAL.print(pressure / 100.0, 2); // convert Pa to hPa and print with 2 d.p.
    PAL_SERIAL.println(" hPa");

    PAL_SERIAL.print("Humidity: ");
    PAL_SERIAL.print(humidity, 2); // 2 d.p.
    PAL_SERIAL.println(" %");

    PAL_SERIAL.println();
    PAL_DELAY(2000);
}