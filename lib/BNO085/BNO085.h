#pragma once

#include "sh2.h"
#include "sh2_SensorValue.h"
#include "sh2_err.h"
#include "PAL.h"

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

// values
#define BNO085_I2C_ADDRESS 0x4A
// additional Activities not listed in SH-2 lib
#define PAC_ON_STAIRS 8 ///< Activity code for being on stairs
#define PAC_OPTION_COUNT 9 ///< The number of current options for the activity classifier

// Modified from: https://github.com/adafruit/Adafruit_BNO08x/blob/master/src/Adafruit_BNO08x.h and https://github.com/adafruit/Adafruit_BNO08x/blob/master/src/Adafruit_BNO08x.cpp
class BNO085 {
    public:
        BNO085(int8_t reset_pin, const uint8_t i2c_address = BNO085_I2C_ADDRESS, const int32_t sensor_id = 0);
        bool begin();
        bool soft_reset();
        void hardware_reset();
        bool was_reset();
        bool enable_report(const sh2_SensorId_t sensor, const uint32_t interval_us = 10000);
        bool get_sensor_event();
        static int i2c_open(sh2_Hal_t* self);
        static void i2c_close(sh2_Hal_t* self);
        static int i2c_read(sh2_Hal_t* self, uint8_t* pBuffer, unsigned int len, uint32_t* t_us);
        static int i2c_write(sh2_Hal_t* self, uint8_t* pBuffer, unsigned int len);
        static void hal_hardware_reset();
        static uint32_t hal_get_time_us(sh2_Hal_t* self);
        static void hal_callback(void* cookie, sh2_AsyncEvent_t* pEvent);
        static void sensor_handler(void* cookie, sh2_SensorEvent_t* event);
        bool wake_up();
        bool sleep();
        static sh2_SensorValue_t* get_sensor_value_ptr();
        
        sh2_ProductIds_t prod_ids; ///< the product IDs returned by the sensor
    
    protected:
        virtual bool init_(int32_t sensor_id);

        sh2_Hal_t HAL_; ///< the struct representing the SH2 Hardware Abstraction Layer
        const int32_t sensor_id_;
        static PAL_TWOWIRE* wire_;
        static uint8_t i2c_address_;
        static int8_t reset_pin_;
        static bool reset_occurred_;
        static sh2_SensorValue_t sensor_value_;
};