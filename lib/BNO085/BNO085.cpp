#include "BNO085.h"

PAL_TWOWIRE* BNO085::wire_ = &PAL_WIRE;
uint8_t BNO085::i2c_address_ = 0xFF;
int8_t BNO085::reset_pin_ = -1;
bool BNO085::reset_occurred_ = false;
sh2_SensorValue_t BNO085::sensor_value_ = {};

BNO085::BNO085(int8_t reset_pin, const uint8_t i2c_address/* = BNO085_I2C_ADDRESS*/, const int32_t sensor_id/* = 0*/) : sensor_id_(sensor_id) {
    i2c_address_ = i2c_address;
    reset_pin_ = reset_pin;
}

bool BNO085::begin() {
    HAL_.open = i2c_open;
    HAL_.close = i2c_close;
    HAL_.read = i2c_read;
    HAL_.write = i2c_write;
    HAL_.getTimeUs = hal_get_time_us;

    return init_(sensor_id_);
}

bool BNO085::init_(const int32_t sensor_id) {
    (void)sensor_id; // make compiler happy

    hardware_reset();

    PAL_DELAY(500);

    // open SH2 interface (also registers non-sensor event handler.)
    int status = sh2_open(&HAL_, hal_callback, nullptr);
    if (status != SH2_OK) {
        return false;
    }

    // check connection partially by getting the product id's
    memset(&prod_ids, 0, sizeof(prod_ids));
    status = sh2_getProdIds(&prod_ids);
    if (status != SH2_OK) {
        return false;
    }

    // register sensor listener
    sh2_setSensorCallback(sensor_handler, nullptr);

    return true;
}

bool BNO085::soft_reset() {
    return sh2_devReset() == SH2_OK;
}

void BNO085::hardware_reset() {
    hal_hardware_reset();
}

bool BNO085::was_reset() {
    const bool reset_occurred = reset_occurred_;
    reset_occurred_ = false;
    return reset_occurred;
}

bool BNO085::enable_report(const sh2_SensorId_t sensor_id, const uint32_t interval_us/* = 10000*/) {
    static sh2_SensorConfig_t config;

    // these sensor options are disabled or not used in most cases
    config.changeSensitivityEnabled = false;
    config.wakeupEnabled = false;
    config.changeSensitivityRelative = false;
    config.alwaysOnEnabled = false;
    config.changeSensitivity = 0;
    config.batchInterval_us = 0;
    config.sensorSpecific = 0;
    config.reportInterval_us = interval_us;

    const int status = sh2_setSensorConfig(sensor_id, &config);

    if (status != SH2_OK) {
        return false;
    }

    return true;
}

bool BNO085::get_sensor_event() {
    sensor_value_.timestamp = 0;

    sh2_service();

    return !((sensor_value_.timestamp == 0) && (sensor_value_.sensorId != SH2_GYRO_INTEGRATED_RV));
}

bool BNO085::wake_up() {
    return sh2_devOn() == SH2_OK;
}

bool BNO085::sleep() {
    return sh2_devSleep() == SH2_OK;
}

sh2_SensorValue_t* BNO085::get_sensor_value_ptr() {
    return &sensor_value_;
}

int BNO085::i2c_open(sh2_Hal_t *self) {
    const uint8_t softreset_pkt[] = {5, 0, 1, 0, 1};
    
    bool success = false;
    for (uint8_t attempts = 0; attempts < 5; attempts++) {
        wire_->beginTransmission(i2c_address_);
        if ((wire_->write(softreset_pkt, ARR_SIZE(softreset_pkt)) == ARR_SIZE(softreset_pkt)) && (wire_->endTransmission() == 0)) {
            success = true;
            break;
        }
        PAL_DELAY(30);
    }

    if (!success) {
        return -1;
    }

    PAL_DELAY(300);

    return 0;
}

void BNO085::i2c_close(sh2_Hal_t* self) {}

int BNO085::i2c_read(sh2_Hal_t* self, uint8_t* pBuffer, unsigned int len, uint32_t* t_us) {
    uint8_t header[4];
    int read_data;
    if (wire_->requestFrom(i2c_address_, ARR_SIZE(header)) != ARR_SIZE(header)) {
        return 0;
    }
    for (uint8_t i = 0; i < ARR_SIZE(header); i++) {
        read_data = wire_->read();
        if (read_data == -1) {
            return 0;
        }
        header[i] = (uint8_t)read_data;
    }

    // determine amount to read
    uint16_t packet_size = (uint16_t)header[0] | ((uint16_t)header[1] << 8);
    packet_size &= ~0x8000; // unset the "continue" bit

    const size_t i2c_buffer_max = wire_->RX_BUFFER_SIZE;

    if (packet_size > len) {
        return 0; // packet wouldn't fit in our buffer
    }

    // the number of non-header bytes to read
    uint16_t cargo_remaining = packet_size;
    uint8_t i2c_buffer[i2c_buffer_max];
    uint16_t read_size;
    uint16_t cargo_read_amount = 0;
    bool first_read = true;

    while (cargo_remaining > 0) {
        if (first_read) {
            read_size = PAL_MIN(i2c_buffer_max, (size_t)cargo_remaining);
        } else {
            read_size = PAL_MIN(i2c_buffer_max, (size_t)cargo_remaining + 4);
        }

        int read_data;
        if (wire_->requestFrom(i2c_address_, read_size) != read_size) {
            return 0;
        }
        for (uint16_t i = 0; i < read_size; i++) {
            read_data = wire_->read();
            if (read_data == -1) {
                return 0;
            }
            i2c_buffer[i] = (uint8_t)read_data;
        }

        if (first_read) {
            // the first time we're saving the "original" header, so include it in the cargo count
            cargo_read_amount = read_size;
            memcpy(pBuffer, i2c_buffer, cargo_read_amount);
            first_read = false;
        } else {
            // this is not the first read, so copy from 4 bytes after the beginning of the i2c buffer to skip the header included with every new i2c read and don't include the header in the amount of cargo read
            cargo_read_amount = read_size - 4;
            memcpy(pBuffer, i2c_buffer + 4, cargo_read_amount);
        }
        // advance our pointer by the amount of cargo read
        pBuffer += cargo_read_amount;
        // mark the caro as received
        cargo_remaining -= cargo_read_amount;
    }

    return packet_size;
}

int BNO085::i2c_write(sh2_Hal_t* self, uint8_t* pBuffer, unsigned int len) {
    const size_t i2c_buffer_max = wire_->TX_BUFFER_SIZE;

    uint16_t write_size = PAL_MIN(i2c_buffer_max, len);
    wire_->beginTransmission(i2c_address_);
    if ((wire_->write(pBuffer, write_size) != write_size) || (wire_->endTransmission() != 0)) {
        return 0;
    }

    return write_size;
}

void BNO085::hal_hardware_reset() {
    if (reset_pin_ == -1) {
        return;
    }
    #if PLATFORM == STM32
        GPIO_InitTypeDef GPIO_init = {};

        const uint16_t GPIO_pin = get_GPIO_pin(reset_pin_);
        GPIO_TypeDef* GPIO_port = get_GPIO_port(reset_pin_);
        
        // GPIO Ports Clock Enable
        if (GPIO_port == GPIOA) {
            __HAL_RCC_GPIOA_CLK_ENABLE();
        } else if (GPIO_port == GPIOB) {
            __HAL_RCC_GPIOB_CLK_ENABLE();
        } else if (GPIO_port == GPIOC) {
            __HAL_RCC_GPIOC_CLK_ENABLE();
        } else if (GPIO_port == GPIOD) {
            __HAL_RCC_GPIOD_CLK_ENABLE();
        } else if (GPIO_port == GPIOF) {
            __HAL_RCC_GPIOF_CLK_ENABLE();
        } else {
            return;
        }

        GPIO_init.Pin = GPIO_pin;
        GPIO_init.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_init.Pull = GPIO_NOPULL;
        GPIO_init.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIO_port, &GPIO_init);
    
        HAL_GPIO_WritePin(GPIO_port, GPIO_pin, GPIO_PIN_SET);
        HAL_Delay(10);

        HAL_GPIO_WritePin(GPIO_port, GPIO_pin, GPIO_PIN_RESET);
        HAL_Delay(10);

        HAL_GPIO_WritePin(GPIO_port, GPIO_pin, GPIO_PIN_SET);
        HAL_Delay(10);
    #elif PLATFORM == ARDUINO
        pinMode(reset_pin_, OUTPUT);
        digitalWrite(reset_pin_, HIGH);
        delay(10);
        digitalWrite(reset_pin_, LOW);
        delay(10);
        digitalWrite(reset_pin_, HIGH);
        delay(10);
    #endif
}

uint32_t BNO085::hal_get_time_us(sh2_Hal_t *self) {
    return PAL_MILLISECONDS() * 1000;
}

void BNO085::hal_callback(void* cookie, sh2_AsyncEvent_t* pEvent) {
    // if we see a reset, set a flag so that sensors will be reconfigured
    if (pEvent->eventId == SH2_RESET) {
        reset_occurred_ = true;
    }
}

// handle sensor events
void BNO085::sensor_handler(void* cookie, sh2_SensorEvent_t* event) {
    const int rc = sh2_decodeSensorEvent(&sensor_value_, event);
    if (rc != SH2_OK) {
        sensor_value_.timestamp = 0;
        return;
    }
}