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
    
    if (!rtc.begin()) {
        PAL_SERIAL.println("RTC initialisation failed");
    }

    PAL_SERIAL.print("RTC begin status: ");
    PAL_SERIAL.println(rtc.begin() ? "true" : "false");

    PAL_SERIAL.print("RTC fetch status: ");
    PAL_SERIAL.println(rtc.fetch() ? "true" : "false");
    PAL_SERIAL.print("Current time: ");
    print_time();

    rtc.enable_clock_output();

    if (!rtc.enable_calibration_output()) {
        PAL_SERIAL.println("Enabling RTC calibration output failed");
    }

    PAL_SERIAL.print("Alarm status: ");
    PAL_SERIAL.println(rtc.set_alarm(10, 0, 0, true, true, true) ? "true" : "false");
}

void loop() {
    rtc.fetch();
    PAL_SERIAL.print("Current time: ");
    print_time();

    while (PAL_SERIAL.available()) {
        PAL_SERIAL.write(PAL_SERIAL.read());
    }

    if (rtc.get_second() == 0) {
        // PAL_SLEEP();
        PAL_STM32_SLEEP();
    }

    PAL_DELAY(10);
}