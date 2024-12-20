#include "DS18B20.h"

DS18B20::DS18B20(const uint8_t physical_pin) : DS18B20_Base(physical_pin) {}

bool DS18B20::init() {
    DS18B20_Base::begin();
    return (bool)DS18B20_Base::selectNext();
}

bool DS18B20::init(const uint8_t address[8]) {
    return (bool)DS18B20_Base::select(const_cast<uint8_t*>(address));
}

float DS18B20::read_temp() {
    return (float)DS18B20_Base::getTempC();
}