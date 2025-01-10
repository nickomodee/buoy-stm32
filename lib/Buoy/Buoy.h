#pragma once

#include <cstdint>
#include <type_traits>
#include <array>
#include "../Sleep/Sleep.h"
#include "../interfaces/DataSender.h"
#include "../BNO085/BNO085.h"
#include "../BatteryVoltageReader/BatteryVoltageReader.h"
#include "../interfaces/TempSensor.h"
#include "../interfaces/HumiditySensor.h"
#include "../interfaces/PressureSensor.h"
#include "../interfaces/UVSensor.h"
#include "../interfaces/VisibleLightSensor.h"
#include "../interfaces/IRLightSensor.h"
#include "../RealTimeClock/RealTimeClock.h"
#include "../SD_File/SD_File.h"
#include "../Half/Half.h"
#include "../Watchdog/Watchdog.h"

using firmware_version_type = float;
constexpr uint8_t firmware_version_type_size = sizeof(firmware_version_type);

using battery_voltage_type = std::invoke_result_t<decltype(BatteryVoltageReader::read_voltage)>;
using temp_sensor_type = std::invoke_result_t<decltype(&TempSensor::read_temp), TempSensor>;
using humidity_sensor_type = std::invoke_result_t<decltype(&HumiditySensor::read_humidity), HumiditySensor>;
using pressure_sensor_type = std::invoke_result_t<decltype(&PressureSensor::read_pressure), PressureSensor>;
using uv_sensor_type = std::invoke_result_t<decltype(&UVSensor::read_uv_index), UVSensor>;
using visible_light_sensor_type = std::invoke_result_t<decltype(&VisibleLightSensor::read_visible), VisibleLightSensor>;
using ir_light_sensor_type = std::invoke_result_t<decltype(&IRLightSensor::read_ir), IRLightSensor>;

constexpr uint8_t battery_voltage_type_size = sizeof(battery_voltage_type);
constexpr uint8_t temp_sensor_type_size = sizeof(temp_sensor_type);
constexpr uint8_t humidity_sensor_type_size = sizeof(humidity_sensor_type);
constexpr uint8_t pressure_sensor_type_size = sizeof(pressure_sensor_type);
constexpr uint8_t uv_sensor_type_size = sizeof(uv_sensor_type);
constexpr uint8_t visible_light_sensor_type_size = sizeof(visible_light_sensor_type);
constexpr uint8_t ir_light_sensor_type_size = sizeof(ir_light_sensor_type);

using second_type = std::invoke_result_t<decltype(&RealTimeClock::get_second), RealTimeClock>;
using minute_type = std::invoke_result_t<decltype(&RealTimeClock::get_minute), RealTimeClock>;
using hour_type = std::invoke_result_t<decltype(&RealTimeClock::get_hour), RealTimeClock>;
using day_type = std::invoke_result_t<decltype(&RealTimeClock::get_day), RealTimeClock>;
using dayofweek_type = std::invoke_result_t<decltype(&RealTimeClock::get_dayofweek), RealTimeClock>;
using month_type = std::invoke_result_t<decltype(&RealTimeClock::get_month), RealTimeClock>;
using year_type = std::invoke_result_t<decltype(&RealTimeClock::get_year), RealTimeClock>;

constexpr uint8_t second_type_size = sizeof(second_type);
constexpr uint8_t minute_type_size = sizeof(minute_type);
constexpr uint8_t hour_type_size = sizeof(hour_type);
constexpr uint8_t day_type_size = sizeof(day_type);
constexpr uint8_t dayofweek_type_size = sizeof(dayofweek_type);
constexpr uint8_t month_type_size = sizeof(month_type);
constexpr uint8_t year_type_size = sizeof(year_type);

using imu_count_type = uint32_t;
using rotation_data_type = Half;
using acceleration_data_type = Half;

constexpr uint8_t imu_count_type_size = sizeof(imu_count_type);
constexpr uint8_t rotation_data_type_size = sizeof(rotation_data_type);
constexpr uint8_t acceleration_data_type_size = sizeof(acceleration_data_type);

using status_type = bool;
constexpr uint8_t status_type_size = sizeof(status_type);

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
class Buoy {
    public:
        Buoy(const uint8_t init_retries, const firmware_version_type firmware_version, DataSender* data_sender, BNO085* bno085, BatteryVoltageReader* battery_voltage_reader, const std::array<TempSensor*, num_temp_sensors>& temp_sensors_array, const std::array<HumiditySensor*, num_humidity_sensors>& humidity_sensors_array, const std::array<PressureSensor*, num_pressure_sensors>& pressure_sensors_array, const std::array<UVSensor*, num_uv_sensors>& uv_sensors_array, const std::array<VisibleLightSensor*, num_visible_light_sensors>& visible_light_sensors_array, const std::array<IRLightSensor*, num_ir_light_sensors>& ir_light_sensors_array);
        bool init_rtc();
        void check_and_sleep();
        void prepare_for_sleep();
        bool init_sd();
        bool init();
        bool record_data();
        bool send_data();

    private:
        static constexpr uint32_t IMU_RECORD_DURATION_ = 0;//30000; // in ms
        static constexpr uint8_t IMU_RECORD_FREQUENCY_ = 10; // in Hertz
        static constexpr char IMU_RECORD_FILE_PATH_[] = "imu_data.bin";

        static constexpr uint8_t WAKE_PERIOD_ = 1; // in minutes
        static constexpr uint8_t MINUTES_BEFORE_ALARM_THRESHOLD_ = 0; // in minutes
        static constexpr uint32_t MILLISECONDS_PER_SECOND_ = 1000; // in ms per second
        static constexpr uint32_t MILLISECONDS_PER_MINUTE_ = 60000; // in ms per minute
        static constexpr int16_t WAKE_OFFSET_ = -(int16_t)(IMU_RECORD_DURATION_ / MILLISECONDS_PER_SECOND_); // in seconds

        const uint8_t init_retries_;
        const firmware_version_type firmware_version_;
        DataSender* data_sender_;
        BNO085* bno085_;
        BatteryVoltageReader* battery_voltage_reader_;
        const std::array<TempSensor*, num_temp_sensors>& temp_sensors_array_;
        const std::array<HumiditySensor*, num_humidity_sensors>& humidity_sensors_array_;
        const std::array<PressureSensor*, num_pressure_sensors>& pressure_sensors_array_;
        const std::array<UVSensor*, num_uv_sensors>& uv_sensors_array_;
        const std::array<VisibleLightSensor*, num_visible_light_sensors>& visible_light_sensors_array_;
        const std::array<IRLightSensor*, num_ir_light_sensors>& ir_light_sensors_array_;

        status_type imu_status_;
        status_type imu_file_status_;
        status_type imu_sleep_status_;
        status_type rtc_status_;
        status_type sd_status_;
        status_type battery_voltage_reader_status_;
        std::array<status_type, num_temp_sensors> temp_sensors_status_array_ = {};
        std::array<status_type, num_humidity_sensors> humidity_sensors_status_array_ = {};
        std::array<status_type, num_pressure_sensors> pressure_sensors_status_array_ = {};
        std::array<status_type, num_uv_sensors> uv_sensors_status_array_ = {};
        std::array<status_type, num_visible_light_sensors> visible_light_sensors_status_array_ = {};
        std::array<status_type, num_ir_light_sensors> ir_light_sensors_status_array_ = {};

        //                                                      [Firmware Version]   ->    [SD Status]   ->    [RTC Status]   ->    [Second]    ->     [Minute]    ->     [Hour]    ->     [Day]    ->    [Day of Week]    ->    [Month]    ->     [Year] ->  [Battery Voltage Status] ->  [Battery Voltage]     ->      [Num Temp Sensors] -> [Temp Sensor 1 Status] -> [Temp Sensor 1 Data]  ->   ...      ->       [Num Humidity Sensors] -> [Humidity Sensor 1 Status] -> [Humidity Sensor 1 Data]  ->   ...       ->        [Num Pressure Sensors] -> [Pressure Sensor 1 Status] -> [Pressure Sensor 1 Data]  ->   ...       ->        [Num UV Sensors] ->  [UV Sensor 1 Status] -> [UV Sensor 1 Data]  ->   ...       ->       [Num Visible Light Sensors] -> [Visible Light Sensor 1 Status] -> [Visible Light Sensor 1 Data]  ->   ...        ->         [Num IR Light Sensors]  ->  [IR Light Sensor 1 Status] -> [IR Light Sensor 1 Data]  ->   ...
        static constexpr size_t SENSOR_DATA_BUFFER_SIZE_ = firmware_version_type_size + status_type_size + (status_type_size + second_type_size + minute_type_size + hour_type_size + day_type_size + dayofweek_type_size + month_type_size + year_type_size) + (status_type_size  +  battery_voltage_type_size) + ((sizeof(num_temp_sensors) + (status_type_size + temp_sensor_type_size) * num_temp_sensors) + (sizeof(num_humidity_sensors)  +  (status_type_size + humidity_sensor_type_size) * num_humidity_sensors) + (sizeof(num_pressure_sensors)  +  (status_type_size + pressure_sensor_type_size) * num_pressure_sensors) + (sizeof(num_uv_sensors) + (status_type_size + uv_sensor_type_size) * num_uv_sensors)  +  (sizeof(num_visible_light_sensors)   +   (status_type_size + visible_light_sensor_type_size) * num_visible_light_sensors) + (sizeof(num_ir_light_sensors)   +   (status_type_size + ir_light_sensor_type_size) * num_ir_light_sensors));
        static constexpr size_t DATA_BUFFER_SIZE_ = SENSOR_DATA_BUFFER_SIZE_;
        uint8_t data_buffer_[DATA_BUFFER_SIZE_] = {};

        uint8_t calculate_minute_to_wake_();
        uint8_t calculate_second_to_wake_();
        bool set_imu_reports_();
        bool init_imu_();
        bool init_battery_voltage_reader_();
        template <typename SensorType, const uint8_t num_sensors> bool initialise_sensors_(const std::array<SensorType*, num_sensors>& sensors_array, std::array<status_type, num_sensors>& sensors_status_array);
        bool init_temperature_sensors_();
        bool init_humidity_sensors_();
        bool init_pressure_sensors_();
        bool init_uv_sensors_();
        bool init_visible_light_sensors_();
        bool init_ir_light_sensors_();
        
        bool record_imu_data_(size_t* data_buffer_index);
        bool record_imu_sleep_(size_t* data_buffer_index);
        bool record_firmware_version_(size_t* data_buffer_index);
        bool record_rtc_(size_t* data_buffer_index);
        bool record_sd_(size_t* data_buffer_index);
        bool record_battery_voltage_(size_t* data_buffer_index);
        template <typename SensorType, typename sensor_read_func_return_type, const uint8_t num_sensors> bool record_sensors_(size_t* data_buffer_index, const std::array<SensorType*, num_sensors>& sensors_array, sensor_read_func_return_type (SensorType::*sensor_read_func)(), std::array<status_type, num_sensors>& sensors_status_array);
        bool record_temp_sensors_(size_t* data_buffer_index);
        bool record_humidity_sensors_(size_t* data_buffer_index);
        bool record_pressure_sensors_(size_t* data_buffer_index);
        bool record_uv_sensors_(size_t* data_buffer_index);
        bool record_visible_light_sensors_(size_t* data_buffer_index);
        bool record_ir_light_sensors_(size_t* data_buffer_index);
        bool append_to_data_buffer_(size_t* data_buffer_index, const void* memory_address, const uint8_t memory_size);
};

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::Buoy(const uint8_t init_retries, const firmware_version_type firmware_version, DataSender* data_sender, BNO085* bno085, BatteryVoltageReader* battery_voltage_reader, const std::array<TempSensor*, num_temp_sensors>& temp_sensors_array, const std::array<HumiditySensor*, num_humidity_sensors>& humidity_sensors_array, const std::array<PressureSensor*, num_pressure_sensors>& pressure_sensors_array, const std::array<UVSensor*, num_uv_sensors>& uv_sensors_array, const std::array<VisibleLightSensor*, num_visible_light_sensors>& visible_light_sensors_array, const std::array<IRLightSensor*, num_ir_light_sensors>& ir_light_sensors_array) : init_retries_(init_retries), firmware_version_(firmware_version), data_sender_(data_sender), bno085_(bno085), battery_voltage_reader_(battery_voltage_reader), temp_sensors_array_(temp_sensors_array), humidity_sensors_array_(humidity_sensors_array), pressure_sensors_array_(pressure_sensors_array), uv_sensors_array_(uv_sensors_array), visible_light_sensors_array_(visible_light_sensors_array), ir_light_sensors_array_(ir_light_sensors_array) {}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_rtc() {
    rtc_status_ = false;
    for (uint8_t i = 0; i < init_retries_; i++) {
        watchdog.refresh();
        if (rtc.begin()) {
            rtc_status_ = true;
            break;
        }
    }
    return rtc_status_;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
void Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::check_and_sleep() {
    sleep.check_and_sleep();
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
void Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::prepare_for_sleep() {
    const uint8_t minute_to_wake = calculate_minute_to_wake_();
    const uint8_t second_to_wake = calculate_second_to_wake_();
    sleep.prepare_for_sleep(second_to_wake, minute_to_wake, 0, true, false); // `0` because we are masking the hour so the value doesn't matter, `true` because we are masking the hour as we only care about the minute and second, `false` because we don't want to mask the minute
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
uint8_t Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::calculate_minute_to_wake_() {
    if ((!rtc_status_) || (!rtc.fetch())) {
        return 0; // I don't think this can ever happen but if it does we just wake on the hour
    }

    const uint8_t current_minute = rtc.get_minute();
    // we want to wake up on the next nice (aligned, like :00, :15, :30, :45) interval, but if we are too close to it, skip to the next one (instead of risking that we sleep after the alarm triggers and then we have to wait an hour; very unlikely but still)
    const uint8_t time_to_next_interval = WAKE_PERIOD_ - (current_minute % WAKE_PERIOD_);
    const uint8_t num_intervals_to_wait = (time_to_next_interval > MINUTES_BEFORE_ALARM_THRESHOLD_) ? 1 : 2;
    const uint8_t next_interval = (current_minute / WAKE_PERIOD_) + num_intervals_to_wait;
    const uint8_t aligned_minute_to_wake = (next_interval * WAKE_PERIOD_) % 60;
    const int8_t wake_offset_seconds = WAKE_OFFSET_ % 60;
    const int8_t wake_offset_minutes = (WAKE_OFFSET_ / 60) + ((wake_offset_seconds < 0) ? -1 : 0);
    const uint8_t minute_to_wake = (aligned_minute_to_wake + 60 + wake_offset_minutes) % 60;
    return minute_to_wake;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
uint8_t Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::calculate_second_to_wake_() {
    const int8_t wake_offset_seconds = WAKE_OFFSET_ % 60;
    const uint8_t second_to_wake = (60 + wake_offset_seconds) % 60;
    return second_to_wake;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_sd() {
    sd_status_ = false;
    for (uint8_t i = 0; i < init_retries_; i++) {
        watchdog.refresh();
        if (sd.begin()) {
            sd_status_ = true;
            break;
        }
    }
    return sd_status_;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::set_imu_reports_() {
    if (!bno085_->enable_report(SH2_ROTATION_VECTOR, 10000)) {
        return false;
    }

    return bno085_->enable_report(SH2_LINEAR_ACCELERATION, 10000);
    // return bno085_->enable_report(SH2_ACCELEROMETER, 10000);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_imu_() {
    imu_status_ = false;
    for (uint8_t i = 0; i < init_retries_; i++) {
        watchdog.refresh();
        if (bno085_->begin() && set_imu_reports_()) {
            imu_status_ = true;
            break;
        }
    }
    return imu_status_;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_battery_voltage_reader_() {
    battery_voltage_reader_status_ = false;
    for (uint8_t i = 0; i < init_retries_; i++) {
        watchdog.refresh();
        if (battery_voltage_reader_->init()) {
            battery_voltage_reader_status_ = true;
            break;
        }
    }
    return battery_voltage_reader_status_;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
template <typename SensorType, const uint8_t num_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::initialise_sensors_(const std::array<SensorType*, num_sensors>& sensors_array, std::array<status_type, num_sensors>& sensors_status_array) {
    bool status = true;

    for (size_t i = 0; i < sensors_array.size(); ++i) {
        watchdog.refresh();
        auto sensor = sensors_array[i];
        
        if (!sensor) {
            status = false;
            sensors_status_array[i] = false;
            continue;
        }

        status_type sensor_status = false;
        for (uint8_t i = 0; i < init_retries_; i++) {
            watchdog.refresh();
            if (sensor->init()) {
                sensor_status = true;
                break;
            }
        }
        sensors_status_array[i] = sensor_status;
        status &= sensor_status;
    }

    return status;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_temperature_sensors_() {
    return initialise_sensors_<TempSensor, num_temp_sensors>(temp_sensors_array_, temp_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_humidity_sensors_() {
    return initialise_sensors_<HumiditySensor, num_humidity_sensors>(humidity_sensors_array_, humidity_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_pressure_sensors_() {
    return initialise_sensors_<PressureSensor, num_pressure_sensors>(pressure_sensors_array_, pressure_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_uv_sensors_() {
    return initialise_sensors_<UVSensor, num_uv_sensors>(uv_sensors_array_, uv_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_visible_light_sensors_() {
    return initialise_sensors_<VisibleLightSensor, num_visible_light_sensors>(visible_light_sensors_array_, visible_light_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init_ir_light_sensors_() {
    return initialise_sensors_<IRLightSensor, num_ir_light_sensors>(ir_light_sensors_array_, ir_light_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::init() {
    bool status = true;

    status &= rtc_status_; // status &= init_rtc_(); // we do already this in `setup()`
    status &= sd_status_; // status &= init_sd_(); // we do already this in `setup()`
    // status &= init_imu_();
    status &= init_battery_voltage_reader_();
    status &= init_temperature_sensors_();
    status &= init_humidity_sensors_();
    status &= init_pressure_sensors_();
    status &= init_uv_sensors_();
    status &= init_visible_light_sensors_();
    status &= init_ir_light_sensors_();

    return status;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::append_to_data_buffer_(size_t* data_buffer_index, const void* memory_address, const uint8_t memory_size) {
    if (*data_buffer_index + memory_size > DATA_BUFFER_SIZE_) {
        return false;
    }

    memcpy(data_buffer_ + *data_buffer_index, memory_address, memory_size);
    *data_buffer_index += memory_size;
    return true;
}


template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_imu_data_(size_t* data_buffer_index) {
    if (!append_to_data_buffer_(data_buffer_index, &imu_status_, status_type_size)) {
        return false;
    }

    SD_File file{IMU_RECORD_FILE_PATH_};
    imu_file_status_ = true;
    if (!sd_status_ || !file.open(FA_WRITE | FA_CREATE_ALWAYS)) {
        imu_file_status_ = false;
    }

    if (!append_to_data_buffer_(data_buffer_index, &imu_file_status_, status_type_size)) {
        return false;
    }

    imu_count_type imu_count = 0;
    if (!imu_status_ || !imu_file_status_) {
        file.close();
        return append_to_data_buffer_(data_buffer_index, &imu_count, imu_count_type_size); // even though it's a fail the return value here refers to the success of appending to the data buffer
    }

    rotation_data_type rotation_i{0.0f}, rotation_j{0.0f}, rotation_k{0.0f}, rotation_real{0.0f};
    acceleration_data_type acceleration_x{0.0f}, acceleration_y{0.0f}, acceleration_z{0.0f};
    
    const uint32_t record_period = 1000 / IMU_RECORD_FREQUENCY_;
    const uint32_t start_time = PAL_MILLISECONDS();
    uint32_t last_record_time = start_time;
    while (PAL_MILLISECONDS() - start_time <= IMU_RECORD_DURATION_) {
        watchdog.refresh();
        if (bno085_->was_reset()) {
            set_imu_reports_();
        }
        
        if (bno085_->get_sensor_event()) {
            sh2_SensorValue_t& sensor_value = *bno085_->get_sensor_value_ptr();

            switch (sensor_value.sensorId) {
                case SH2_ROTATION_VECTOR:
                    rotation_i = rotation_data_type{sensor_value.un.rotationVector.i};
                    rotation_j = rotation_data_type{sensor_value.un.rotationVector.j};
                    rotation_k = rotation_data_type{sensor_value.un.rotationVector.k};
                    rotation_real = rotation_data_type{sensor_value.un.rotationVector.real};
                    break;

                case SH2_LINEAR_ACCELERATION:
                    acceleration_x = acceleration_data_type{sensor_value.un.linearAcceleration.x};
                    acceleration_y = acceleration_data_type{sensor_value.un.linearAcceleration.y};
                    acceleration_z = acceleration_data_type{sensor_value.un.linearAcceleration.z};
                    break;
                // case SH2_ACCELEROMETER:
                //     acceleration_x = acceleration_data_type{sensor_value.un.accelerometer.x};
                //     acceleration_y = acceleration_data_type{sensor_value.un.accelerometer.y};
                //     acceleration_z = acceleration_data_type{sensor_value.un.accelerometer.z};
                //     break;
            }
        }

        if (PAL_MILLISECONDS() - last_record_time < record_period) {
            continue;
        }

        file.write((const uint8_t*)&rotation_i, rotation_data_type_size);
        file.write((const uint8_t*)&rotation_j, rotation_data_type_size);
        file.write((const uint8_t*)&rotation_k, rotation_data_type_size);
        file.write((const uint8_t*)&rotation_real, rotation_data_type_size);

        file.write((const uint8_t*)&acceleration_x, acceleration_data_type_size);
        file.write((const uint8_t*)&acceleration_y, acceleration_data_type_size);
        file.write((const uint8_t*)&acceleration_z, acceleration_data_type_size);

        last_record_time += record_period;
    }

    file.close();
    if (!file.open(FA_READ)) {
        return append_to_data_buffer_(data_buffer_index, &imu_count, imu_count_type_size);
    }

    const uint32_t file_size = file.get_size();
    imu_count = file_size / (4 * rotation_data_type_size + 3 * acceleration_data_type_size);
    file.close();

    return append_to_data_buffer_(data_buffer_index, &imu_count, imu_count_type_size);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_imu_sleep_(size_t* data_buffer_index) {
    imu_sleep_status_ = (status_type)bno085_->sleep();
    return append_to_data_buffer_(data_buffer_index, &imu_sleep_status_, status_type_size);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_firmware_version_(size_t* data_buffer_index) {
    return append_to_data_buffer_(data_buffer_index, &firmware_version_, firmware_version_type_size);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_rtc_(size_t* data_buffer_index) {
    second_type second = 0;
    minute_type minute = 0;
    hour_type hour = 0;
    day_type day = 0;
    dayofweek_type dayofweek = 0;
    month_type month = 0;
    year_type year = 0;

    if (rtc_status_ && rtc.fetch()) {
        second = rtc.get_second();
        minute = rtc.get_minute();
        hour = rtc.get_hour();
        day = rtc.get_day();
        dayofweek = rtc.get_dayofweek();
        month = rtc.get_month();
        year = rtc.get_year();
    } else {
        rtc_status_ = false;
    }

    if (!append_to_data_buffer_(data_buffer_index, &rtc_status_, status_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &second, second_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &minute, minute_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &hour, hour_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &day, day_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &dayofweek, dayofweek_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &month, month_type_size)) {
        return false;
    }
    if (!append_to_data_buffer_(data_buffer_index, &year, year_type_size)) {
        return false;
    }

    return true;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_sd_(size_t* data_buffer_index) {
    return append_to_data_buffer_(data_buffer_index, &sd_status_, status_type_size);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_battery_voltage_(size_t* data_buffer_index) {
    if (!append_to_data_buffer_(data_buffer_index, &battery_voltage_reader_status_, status_type_size)) {
        return false;
    }

    const battery_voltage_type battery_voltage = battery_voltage_reader_->read_voltage();
    return append_to_data_buffer_(data_buffer_index, &battery_voltage, battery_voltage_type_size);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
template <typename SensorType, typename sensor_read_func_return_type, const uint8_t num_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_sensors_(size_t* data_buffer_index, const std::array<SensorType*, num_sensors>& sensors_array, sensor_read_func_return_type (SensorType::*sensor_read_func)(), std::array<status_type, num_sensors>& sensors_status_array) {
    const uint8_t num_sensors_value = num_sensors;
    if (!append_to_data_buffer_(data_buffer_index, &num_sensors_value, sizeof(num_sensors_value))) {
        return false;
    }

    bool success = true;
    for (size_t i = 0; i < sensors_array.size(); ++i) {
        watchdog.refresh();
        auto sensor = sensors_array[i];
        
        if (!sensor) {
            success = false;
        }

        const status_type sensor_status = sensors_status_array[i];
        if (!append_to_data_buffer_(data_buffer_index, &sensor_status, status_type_size)) {
            return false;
        }

        const sensor_read_func_return_type sensor_value = ((sensor) && (sensor_status)) ? (sensor->*sensor_read_func)() : sensor_read_func_return_type();
        if (!append_to_data_buffer_(data_buffer_index, &sensor_value, sizeof(sensor_read_func_return_type))) {
            return false;
        }
    }

    return success;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_temp_sensors_(size_t* data_buffer_index) {
    return record_sensors_<TempSensor, temp_sensor_type, num_temp_sensors>(data_buffer_index, temp_sensors_array_, &TempSensor::read_temp, temp_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_humidity_sensors_(size_t* data_buffer_index) {
    return record_sensors_<HumiditySensor, humidity_sensor_type, num_humidity_sensors>(data_buffer_index, humidity_sensors_array_, &HumiditySensor::read_humidity, humidity_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_pressure_sensors_(size_t* data_buffer_index) {
    return record_sensors_<PressureSensor, pressure_sensor_type, num_pressure_sensors>(data_buffer_index, pressure_sensors_array_, &PressureSensor::read_pressure, pressure_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_uv_sensors_(size_t* data_buffer_index) {
    return record_sensors_<UVSensor, uv_sensor_type, num_uv_sensors>(data_buffer_index, uv_sensors_array_, &UVSensor::read_uv_index, uv_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_visible_light_sensors_(size_t* data_buffer_index) {
    return record_sensors_<VisibleLightSensor, visible_light_sensor_type, num_visible_light_sensors>(data_buffer_index, visible_light_sensors_array_, &VisibleLightSensor::read_visible, visible_light_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_ir_light_sensors_(size_t* data_buffer_index) {
    return record_sensors_<IRLightSensor, ir_light_sensor_type, num_ir_light_sensors>(data_buffer_index, ir_light_sensors_array_, &IRLightSensor::read_ir, ir_light_sensors_status_array_);
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::record_data() {
    size_t data_buffer_index = 0;
    // if (!record_imu_data_(&data_buffer_index)) {
    //     return false;
    // }
    // PAL_SERIAL.println(1);
    // if (!record_imu_sleep_(&data_buffer_index)) {
    //     return false;
    // }
    if (!record_firmware_version_(&data_buffer_index)) {
        return false;
    }
    if (!record_sd_(&data_buffer_index)) {
        return false;
    }
    if (!record_rtc_(&data_buffer_index)) {
        return false;
    }
    if (!record_battery_voltage_(&data_buffer_index)) {
        return false;
    }
    if (!record_temp_sensors_(&data_buffer_index)) {
        return false;
    }
    if (!record_humidity_sensors_(&data_buffer_index)) {
        return false;
    }
    if (!record_pressure_sensors_(&data_buffer_index)) {
        return false;
    }
    if (!record_uv_sensors_(&data_buffer_index)) {
        return false;
    }
    if (!record_visible_light_sensors_(&data_buffer_index)) {
        return false;
    }
    if (!record_ir_light_sensors_(&data_buffer_index)) {
        return false;
    }

    return true;
}

template<const uint8_t num_temp_sensors, const uint8_t num_humidity_sensors, const uint8_t num_pressure_sensors, const uint8_t num_uv_sensors, const uint8_t num_visible_light_sensors, const uint8_t num_ir_light_sensors>
bool Buoy<num_temp_sensors, num_humidity_sensors, num_pressure_sensors, num_uv_sensors, num_visible_light_sensors, num_ir_light_sensors>::send_data() {
    return data_sender_->send((const char*)data_buffer_, DATA_BUFFER_SIZE_, (sd_status_ && imu_file_status_) ? IMU_RECORD_FILE_PATH_ : nullptr);
}