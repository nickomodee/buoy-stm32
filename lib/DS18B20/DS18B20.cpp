#include "DS18B20.h"

DS18B20::DS18B20(const uint8_t physical_pin, const uint8_t resolution/* = 12*/) : DS18B20_Base(physical_pin), resolution_(resolution) {}

bool DS18B20::init() {
    DS18B20_Base::begin();
    const bool init_status = (bool)DS18B20_Base::selectNext();
    DS18B20_Base::setResolution(resolution_);
}

bool DS18B20::init(const uint8_t address[8]) {
    return (bool)DS18B20_Base::select(const_cast<uint8_t*>(address));
}

float DS18B20::read_temp() {
    return (float)DS18B20_Base::getTempC();
}