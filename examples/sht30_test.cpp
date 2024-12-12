#include "PAL.h"
#include "SHT30.h"
#include "TempSensor.h"
#include "HumiditySensor.h"

SHT30 sht30;
TempSensor& temp_sensor = sht30;
HumiditySensor& humidity_sensor_bme280 = sht30;

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();
    while (!sht30.init()) {
        PAL_SERIAL.println("SHT30 initialisation failed");
        PAL_DELAY(1000);
    }
}

void loop() {
    const float temperature = temp_sensor.read_temp();
    const float humidity = humidity_sensor_bme280.read_humidity();

    PAL_SERIAL.print("Temperature: ");
    PAL_SERIAL.print(temperature, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    PAL_SERIAL.print("Humidity: ");
    PAL_SERIAL.print(humidity, 2); // 2 d.p.
    PAL_SERIAL.println("%");

    PAL_SERIAL.println();
    PAL_DELAY(2000);
}