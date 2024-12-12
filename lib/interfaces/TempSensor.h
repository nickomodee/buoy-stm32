#pragma once

/**
 * @brief Interface for reading temperature.
 */
class TempSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the temperature.
         * 
         * @returns A float of the value of the temperature in degrees Celsius.
         */
        virtual float read_temp() = 0;
        virtual ~TempSensor() = default;
};