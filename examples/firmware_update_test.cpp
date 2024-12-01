#include "PAL.h"
#include "SD.h"
#include "SD_File.h"
#include "FirmwareUpdater.h"
#include <cstdbool>

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

FirmwareUpdater firmware_updater;

void setup() {
    PAL_SERIAL.begin(9600);
    PAL_DELAY(2000);

    // PAL_SERIAL.println("Update was successful"); // uncomment to test

    uint8_t sd_init_status;
    while (true) {
        sd_init_status = sd.begin();
        if (sd_init_status == 1) {
            break;
        } 

        PAL_SERIAL.print("SD Initialisation Failed, Status: ");
        PAL_SERIAL.println(sd_init_status);
        PAL_DELAY(1000);
    }
    PAL_SERIAL.println("SD Initialised");

    {
        char output[256];
        sd.list_dir("", output, ARR_SIZE(output));
        PAL_SERIAL.println(output);
    }

    const bool firmware_update_available = firmware_updater.check();
    PAL_SERIAL.print("Firmware update available: ");
    PAL_SERIAL.println(firmware_update_available);

    if (firmware_update_available) {
        PAL_SERIAL.println("Updating firmware...");
        PAL_SERIAL.print("Firmware update status: ");
        PAL_SERIAL.println(firmware_updater.update());
        PAL_SERIAL.println("This message shouldn't be seen if successful (unless updating with the same firmware)");
    } else {
        PAL_SERIAL.println("Not updating");
    }
}

void loop() {}