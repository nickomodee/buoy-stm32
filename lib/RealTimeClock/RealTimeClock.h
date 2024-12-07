#pragma once

#include "stm32f3xx_hal.h"

#define RTC_BKP_VALUE 0x32F2
#define RTC_BKP_DR_INDEX RTC_BKP_DR1

class RealTimeClock {
    public:
        RealTimeClock();
        bool begin();
        bool fetch();
        bool set(const uint8_t second, const uint8_t minute, const uint8_t hour, const uint8_t day, const uint8_t dayofweek, const uint8_t month, const uint16_t year);
        bool set_alarm(const uint8_t second, const uint8_t minute, const uint8_t hour, const bool mask_dateweekday = false, const bool mask_hour = false, const bool mask_minute = false);
        bool disable_alarm();
        uint8_t get_second();
        uint8_t get_minute();
        uint8_t get_hour();
        uint8_t get_day();
        uint8_t get_dayofweek();
        uint8_t get_month();
        uint16_t get_year();
        // bool calibrate(const float offset_ppm);
        static void enable_clock_output();
        bool enable_calibration_output();
        bool set_calibrate_alarm();
        void calibrate_alarm_callback();
        void alarm_callback();
        RTC_HandleTypeDef* get_hrtc_ptr();

        static const constexpr uint16_t year_offset = 2000;
    
    private:
        static RTC_TypeDef* RTC_instance_;
        static RTC_HandleTypeDef hrtc_;
        static const constexpr uint32_t alarm_ = RTC_ALARM_A;
        static bool calibrating_;
        static constexpr uint32_t calibrated_asynch_prevdiv_ = 127;
        static constexpr uint32_t calibrated_synch_prevdiv_ = 317; // with calibrated LSI clock frequency of 40740 Hz

        uint8_t second_;
        uint8_t minute_;
        uint8_t hour_;
        uint8_t day_;
        uint8_t dayofweek_;
        uint8_t month_;
        uint16_t year_;
};

extern RealTimeClock rtc;