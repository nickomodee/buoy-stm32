#pragma once

#include "../interfaces/TempSensor.h"
#include "../DS18B20_Base/DS18B20_Base.h"

class DS18B20 : public DS18B20_Base, public TempSensor {
    public:
        DS18B20(const uint8_t physical_pin); // not GPIO pin
        bool init() override; // selects the first device connected
        bool init(const uint8_t address[8]); // selects the device at the address
        double read_temp() override;
};