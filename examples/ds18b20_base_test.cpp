#include "DS18B20_Base.h"

DS18B20_Base ds(7);

void setup() {
    PAL_SERIAL.begin(9600);
    PAL_SERIAL.print("Devices: ");
    PAL_SERIAL.println(ds.getNumberOfDevices());
    PAL_SERIAL.println();
}

void loop() {
    while (ds.selectNext()) {
        switch (ds.getFamilyCode()) {
            case MODEL_DS18S20:
                PAL_SERIAL.println("Model: DS18S20/DS1820");
                break;
            case MODEL_DS1822:
                PAL_SERIAL.println("Model: DS1822");
                break;
            case MODEL_DS18B20:
                PAL_SERIAL.println("Model: DS18B20");
                break;
            default:
                PAL_SERIAL.println("Unrecognized Device");
            break;
        }

        uint8_t address[8];
        ds.getAddress(address);

        PAL_SERIAL.print("Address:");
        for (uint8_t i = 0; i < 8; i++) {
            PAL_SERIAL.print(" ");
            PAL_SERIAL.print(address[i]);
        }
        PAL_SERIAL.println();

        PAL_SERIAL.print("Resolution: ");
        PAL_SERIAL.println(ds.getResolution());

        PAL_SERIAL.print("Power Mode: ");
        if (ds.getPowerMode()) {
            PAL_SERIAL.println("External");
        } else {
            PAL_SERIAL.println("Parasite");
        }

        PAL_SERIAL.print("Temperature: ");
        PAL_SERIAL.print(ds.getTempC());
        PAL_SERIAL.print(" C / ");
        PAL_SERIAL.print(ds.getTempF());
        PAL_SERIAL.println(" F");
        PAL_SERIAL.println();
    }

    PAL_DELAY(2000);
}