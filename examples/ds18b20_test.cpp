#include "PAL.h"
#include "DS18B20.h"
#include "TempSensor.h"

#define ONEWIRE_PIN 7

DS18B20 ds18b20(ONEWIRE_PIN);
TempSensor& temp_sensor = ds18b20;

void setup() {
    PAL_SERIAL.begin(9600);
    while (!ds18b20.init()) {
        PAL_SERIAL.println("DS18B20 initialisation failed");
        PAL_DELAY(1000);
    }
}

void loop() {
    const float temperature = temp_sensor.read_temp();

    PAL_SERIAL.print("Temperature: ");
    PAL_SERIAL.print(temperature, 2); // 2 d.p.
    PAL_SERIAL.println(" °C");

    PAL_SERIAL.println();
    PAL_DELAY(2000);
}