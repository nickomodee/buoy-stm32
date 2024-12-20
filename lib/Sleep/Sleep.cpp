#include "Sleep.h"

Sleep::Sleep() {}

void Sleep::prepare_for_sleep(const uint8_t second, const uint8_t minute, const uint8_t hour, const bool mask_hour/* = false*/, const bool mask_minute/* = false*/) {
    prepare_sleep_alarm_(second, minute, hour, mask_hour, mask_minute);
    enable_rtc_sleep_flag_();
    __disable_irq();
    HAL_NVIC_SystemReset(); // this way the IWDG is disabled on reset before entering standby mode
}

// so this is completely unnecessary as alarms do persist on resets... but I cba to take it out cause it works and we don't need the registers for now
void Sleep::prepare_sleep_alarm_(const uint8_t second, const uint8_t minute, const uint8_t hour, const bool mask_hour/* = false*/, const bool mask_minute/* = false*/) {
    uint32_t alarm_backup_register_value = 0;
    alarm_backup_register_value |= (uint32_t)second;
    alarm_backup_register_value |= ((uint32_t)minute << 8);
    alarm_backup_register_value |= ((uint32_t)hour << 16);
    if (mask_hour) {
        alarm_backup_register_value |= ((uint32_t)1 << 24);
    }
    if (mask_minute) {
        alarm_backup_register_value |= ((uint32_t)1 << 25);
    }
    rtc.write_backup_domain_register(SLEEP_ALARM_BACKUP_DR_INDEX, alarm_backup_register_value);
}

// so this is completely unnecessary as alarms do persist on resets... but I cba to take it out cause it works and we don't need the registers for now
bool Sleep::set_sleep_alarm_() {
    const uint32_t alarm_backup_register_value = rtc.read_backup_domain_register(SLEEP_ALARM_BACKUP_DR_INDEX);
    const uint8_t second = alarm_backup_register_value & 0xFF;
    const uint8_t minute = (alarm_backup_register_value >> 8) & 0xFF;
    const uint8_t hour = (alarm_backup_register_value >> 16) & 0xFF;
    const bool mask_hour = (bool)((alarm_backup_register_value >> 24) & 1);
    const bool mask_minute = (bool)((alarm_backup_register_value >> 25) & 1);

    return rtc.set_alarm(second, minute, hour, true, mask_hour, mask_minute); // the `true` is for masking the weekday
}

void Sleep::check_and_sleep() {
    if ((!is_set_rtc_sleep_flag_()) || (!set_sleep_alarm_())) {
        return;
    }

    disable_rtc_sleep_flag_(); // so that after the reset from standby mode we continue
    PAL_STM32_SLEEP();
}

bool Sleep::is_set_rtc_sleep_flag_() {
    return rtc.read_backup_domain_register(SLEEP_BACKUP_DR_INDEX) == SLEEP_BACKUP_DR_VALUE;
}

void Sleep::enable_rtc_sleep_flag_() {
    rtc.write_backup_domain_register(SLEEP_BACKUP_DR_INDEX, SLEEP_BACKUP_DR_VALUE);
}

void Sleep::disable_rtc_sleep_flag_() {
    rtc.write_backup_domain_register(SLEEP_BACKUP_DR_INDEX, 0);
}

Sleep sleep;