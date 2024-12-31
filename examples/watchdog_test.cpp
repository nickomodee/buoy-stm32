#include "PAL.h"
#include "Watchdog.h"

constexpr uint32_t SLEEP_TIME = 60000;

void setup() {
    const bool watchdog_initialisation_status = watchdog.init();

    PAL_SERIAL.begin(9600);
    
    PAL_SERIAL.print("Watchdog initialisation status: ");
    PAL_SERIAL.println(watchdog_initialisation_status ? "true" : "false");
}

void loop() {
    PAL_SERIAL.println("Looping for 60 seconds with refresh...");
    uint32_t start_time = PAL_MILLISECONDS();
    while (PAL_MILLISECONDS() - start_time < SLEEP_TIME) {
        watchdog.refresh();
    }

    PAL_SERIAL.println("Looping for 60 seconds without refresh...");
    start_time = PAL_MILLISECONDS();
    while (PAL_MILLISECONDS() - start_time < SLEEP_TIME) {}
}