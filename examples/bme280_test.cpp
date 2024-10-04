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
    Serial.begin(9600);
    Wire.begin();
    while (!bme280.init()) {
        PAL_DELAY(1000);
    }
}

void loop() {
    float temperature = temp_sensor.read_temp();
    float pressure = pressure_sensor.read_pressure();
    float humidity = humidity_sensor.read_humidity();

    Serial.print("Temperature: ");
    Serial.print(temperature, 2); // 2 d.p.
    Serial.println(" °C");

    Serial.print("Pressure: ");
    Serial.print(pressure / 100.0, 2); // convert Pa to hPa and print with 2 d.p.
    Serial.println(" hPa");

    Serial.print("Humidity: ");
    Serial.print(humidity, 2); // 2 d.p.
    Serial.println(" %");

    Serial.println();
    PAL_DELAY(2000);
}