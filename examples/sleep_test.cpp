#include "Sleep.h"
#include "RealTimeClock.h"
#include "PAL.h"

void print_time() {
    PAL_SERIAL.print(rtc.get_day());
    PAL_SERIAL.print("/");
    PAL_SERIAL.print(rtc.get_month());
    PAL_SERIAL.print("/");
    PAL_SERIAL.print(rtc.get_year());
    PAL_SERIAL.print(" ");
    PAL_SERIAL.print(rtc.get_hour());
    PAL_SERIAL.print(" ");
    PAL_SERIAL.print(rtc.get_minute());
    PAL_SERIAL.print(" ");
    PAL_SERIAL.println(rtc.get_second());
}

void setup() {
    PAL_SERIAL.begin(9600);

    PAL_SERIAL.print("RTC begin status: ");
    PAL_SERIAL.println(rtc.begin() ? "true" : "false");
    
    sleep.check_and_sleep();

    PAL_SERIAL.print("RTC fetch status: ");
    PAL_SERIAL.println(rtc.fetch() ? "true" : "false");
    PAL_SERIAL.print("Current time: ");
    print_time();
}

void loop() {
    rtc.fetch();
    PAL_SERIAL.print("Current time: ");
    print_time();

    while (PAL_SERIAL.available()) {
        PAL_SERIAL.write(PAL_SERIAL.read());
    }

    if (rtc.get_second() == 0) {
        sleep.prepare_for_sleep(10, 0, 0, true, true);
    }

    PAL_DELAY(10);
}