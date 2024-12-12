#include "PAL.h"
#include "BNO085.h"

#define BNO085_RESET 11

BNO085 bno085(BNO085_RESET);

void set_reports();

void setup() {
    PAL_SERIAL.begin(115200);
    Wire.begin();
    PAL_SERIAL.println("BNO085 test");

    if (!bno085.begin()) {
        PAL_SERIAL.println("Failed to find BNO085 chip");
        while (1) {
            PAL_DELAY(10);
        }
    }
    PAL_SERIAL.println("BNO085 found");

    for (int n = 0; n < bno085.prod_ids.numEntries; n++) {
        PAL_SERIAL.print("Part ");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swPartNumber);
        PAL_SERIAL.print(": Version :");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swVersionMajor);
        PAL_SERIAL.print(".");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swVersionMinor);
        PAL_SERIAL.print(".");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swVersionPatch);
        PAL_SERIAL.print(" Build ");
        PAL_SERIAL.println(bno085.prod_ids.entry[n].swBuildNumber);
    }

    set_reports();

    PAL_SERIAL.println("Reading events");
    PAL_DELAY(100);
}

// here is where you define the sensor outputs you want to receive
void set_reports() {
    PAL_SERIAL.println("Setting desired reports");
    if (!bno085.enable_report(SH2_GAME_ROTATION_VECTOR)) {
        PAL_SERIAL.println("Could not enable game vector");
    }
}

void loop() {
    if (bno085.was_reset()) {
        PAL_SERIAL.print("Sensor was reset ");
        set_reports();
    }

    if (!bno085.get_sensor_event()) {
        return;
    }

    sh2_SensorValue_t& sensor_value = *bno085.get_sensor_value_ptr();

    switch (sensor_value.sensorId) {
        case SH2_GAME_ROTATION_VECTOR: {
            float i = sensor_value.un.gameRotationVector.i;
            float j = sensor_value.un.gameRotationVector.j;
            float k = sensor_value.un.gameRotationVector.k;
            float real = sensor_value.un.gameRotationVector.real;

            // send raw bytes of the floats in little endian format
            PAL_SERIAL.write((uint8_t*)&i, sizeof(float)); 
            PAL_SERIAL.write((uint8_t*)&j, sizeof(float));
            PAL_SERIAL.write((uint8_t*)&k, sizeof(float));
            PAL_SERIAL.write((uint8_t*)&real, sizeof(float));

            break;
        }
    }
}