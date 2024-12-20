#include "RealTimeClock.h"
#include "PAL.h"

RTC_TypeDef* RealTimeClock::RTC_instance_ = RTC;
RTC_HandleTypeDef RealTimeClock::hrtc_;
bool RealTimeClock::calibrating_ = false;

RealTimeClock::RealTimeClock() {}

bool RealTimeClock::begin() {
    HAL_PWR_EnableBkUpAccess(); // enable access to the backup domain
    
    hrtc_.Instance = RTC_instance_;
    hrtc_.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc_.Init.AsynchPrediv = calibrated_asynch_prevdiv_;
    hrtc_.Init.SynchPrediv = calibrated_synch_prevdiv_;
    hrtc_.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc_.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc_.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    __HAL_RCC_RTC_ENABLE();

    if (HAL_RTC_Init(&hrtc_) != HAL_OK) {
        return false;
    }

    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);

    return true;
}

uint32_t RealTimeClock::read_backup_domain_register(const uint32_t backup_register) {
    return HAL_RTCEx_BKUPRead(&hrtc_, backup_register);
}

void RealTimeClock::write_backup_domain_register(const uint32_t backup_register, const uint32_t data) {
    return HAL_RTCEx_BKUPWrite(&hrtc_, backup_register, data);
}

bool RealTimeClock::fetch() {
    RTC_TimeTypeDef time = {};
    if (HAL_RTC_GetTime(&hrtc_, &time, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    second_ = time.Seconds;
    minute_ = time.Minutes;
    hour_ = time.Hours;
    
    RTC_DateTypeDef date = {};
    if (HAL_RTC_GetDate(&hrtc_, &date, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    day_ = date.Date;
    dayofweek_ = date.WeekDay;
    month_ = date.Month;
    year_ = year_offset + date.Year;

    return true;
}

bool RealTimeClock::set(const uint8_t second, const uint8_t minute, const uint8_t hour, const uint8_t day, const uint8_t dayofweek, const uint8_t month, const uint16_t year) {
    RTC_TimeTypeDef time = {};
    time.Seconds = second;
    time.Minutes = minute;
    time.Hours = hour;
    time.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    time.StoreOperation = RTC_STOREOPERATION_RESET;
    if (HAL_RTC_SetTime(&hrtc_, &time, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }

    RTC_DateTypeDef date = {};
    date.Date = day;
    date.WeekDay = dayofweek;
    date.Month = month;
    date.Year = year - year_offset;

    return HAL_RTC_SetDate(&hrtc_, &date, RTC_FORMAT_BIN) == HAL_OK;
}

bool RealTimeClock::is_alarm_set() {
    return __HAL_RTC_ALARM_GET_FLAG(&hrtc_, (alarm_ == RTC_ALARM_A) ? RTC_FLAG_ALRAF : RTC_FLAG_ALRBF);
}

bool RealTimeClock::set_alarm(const uint8_t second, const uint8_t minute, const uint8_t hour, const bool mask_dateweekday/* = false*/, const bool mask_hour/* = false*/, const bool mask_minute/* = false*/) {
    RTC_AlarmTypeDef alarm = {};
    alarm.AlarmTime.Seconds = second;
    alarm.AlarmTime.Minutes = minute;
    alarm.AlarmTime.Hours = hour;
    alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    alarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;

    alarm.AlarmMask = RTC_ALARMMASK_NONE;
    if (mask_dateweekday) {
        alarm.AlarmMask |= RTC_ALARMMASK_DATEWEEKDAY;
    }
    if (mask_hour) {
        alarm.AlarmMask |= RTC_ALARMMASK_HOURS;
    }
    if (mask_minute) {
        alarm.AlarmMask |= RTC_ALARMMASK_MINUTES;
    }
    
    alarm.Alarm = alarm_;
    return HAL_RTC_SetAlarm_IT(&hrtc_, &alarm, RTC_FORMAT_BIN) == HAL_OK;
}

bool RealTimeClock::disable_alarm() {
    return HAL_RTC_DeactivateAlarm(&hrtc_, alarm_) == HAL_OK;
}

uint8_t RealTimeClock::get_second() {
    return second_;
}

uint8_t RealTimeClock::get_minute() {
    return minute_;
}

uint8_t RealTimeClock::get_hour() {
    return hour_;
}

uint8_t RealTimeClock::get_day() {
    return day_;
}

uint8_t RealTimeClock::get_dayofweek() {
    return dayofweek_;
}

uint8_t RealTimeClock::get_month() {
    return month_;
}

uint16_t RealTimeClock::get_year() {
    return year_;
}

void RealTimeClock::enable_clock_output() {
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSI, RCC_MCODIV_1);
}

bool RealTimeClock::enable_calibration_output() {
    calibrating_ = true;
    GPIO_InitTypeDef GPIO_InitStruct = {};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
    
    set_calibrate_alarm();
    
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
    
    return true;
}

bool RealTimeClock::set_calibrate_alarm() {
    RTC_TimeTypeDef currentTime = {};
    if (HAL_RTC_GetTime(&hrtc_, &currentTime, RTC_FORMAT_BIN) != HAL_OK) {
        return false;
    }
    currentTime.Seconds += 2;
    currentTime.Seconds %= 60;
    
    RTC_AlarmTypeDef alarm = {};
    alarm.Alarm = alarm_;
    alarm.AlarmTime.Hours = 0;
    alarm.AlarmTime.Minutes = 0;
    alarm.AlarmTime.Seconds = currentTime.Seconds;
    alarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    alarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    alarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;

    return HAL_RTC_SetAlarm_IT(&hrtc_, &alarm, RTC_FORMAT_BIN) == HAL_OK;
}

RTC_HandleTypeDef* RealTimeClock::get_hrtc_ptr() {
    return &hrtc_;
}

void RealTimeClock::calibrate_alarm_callback() {
    if (!calibrating_) {
        return;
    }

    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
    set_calibrate_alarm();
}

void RealTimeClock::alarm_callback() {
    if (calibrating_) {
        calibrate_alarm_callback();
    }

    // nothing to do if not calibrating
}

extern "C" void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
    (void)hrtc;
    rtc.alarm_callback();
}

extern "C" void RTC_Alarm_IRQHandler() {
    HAL_RTC_AlarmIRQHandler(rtc.get_hrtc_ptr());
}

RealTimeClock rtc;