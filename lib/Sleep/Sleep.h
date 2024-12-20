#pragma once

#include "../RealTimeClock/RealTimeClock.h"
#include "../PAL/PAL.h"

#define SLEEP_BACKUP_DR_INDEX RTC_BKP_DR2
#define SLEEP_BACKUP_DR_VALUE 0x7E4ABC07 // 0xYEAHBUOY?
#define SLEEP_ALARM_BACKUP_DR_INDEX RTC_BKP_DR3

class Sleep {
    public:
        Sleep();
        void prepare_for_sleep(const uint8_t second, const uint8_t minute, const uint8_t hour, const bool mask_hour = false, const bool mask_minute = false);
        void check_and_sleep();
    
    private:
        void prepare_sleep_alarm_(const uint8_t second, const uint8_t minute, const uint8_t hour, const bool mask_hour = false, const bool mask_minute = false);
        bool set_sleep_alarm_();
        bool is_set_rtc_sleep_flag_();
        void enable_rtc_sleep_flag_();
        void disable_rtc_sleep_flag_();
};

extern Sleep sleep;