#include "PAL.h"
#include "BatteryVoltageReader.h"

BatteryVoltageReader battery_voltage_reader;

void setup() {
    PAL_SERIAL.begin(9600);

    while (!battery_voltage_reader.init()) {
        PAL_SERIAL.println("Battery battery_voltage initialisation failed");
        PAL_DELAY(1000);
    }
}

void loop() {
    const float raw_battery_voltage = battery_voltage_reader.read_raw_voltage();
    const float battery_voltage = battery_voltage_reader.read_voltage();

    PAL_SERIAL.print("Raw battery battery_voltage: ");
    PAL_SERIAL.println(raw_battery_voltage, 2); // 2 d.p.
    
    PAL_SERIAL.print("Battery battery_voltage: ");
    PAL_SERIAL.println(battery_voltage, 2); // 2 d.p.

    PAL_SERIAL.println();
    PAL_DELAY(2000);
}