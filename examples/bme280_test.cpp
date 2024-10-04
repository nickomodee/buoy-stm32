#include "PAL.h"
#include "BME280.h"
#include "TempSensor.h"
#include "PressureSensor.h"
#include "HumiditySensor.h"

BME280 bme280;
TempSensor& temp_sensor = bme280;
PressureSensor& pressure_sensor = bme280;
HumiditySensor& humidity_sensor = bme280;

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();
    while (!bme280.init()) {
        PAL_DELAY(1000);
    }
}

void loop() {
    float temperature = temp_sensor.read_temp();
    float pressure = pressure_sensor.read_pressure();
    float humidity = humidity_sensor.read_humidity();

    PAL_SERIAL.print("Temperature: ");
    PAL_SERIAL.print(temperature, 2); // 2 d.p.
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