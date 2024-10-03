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
         * @returns A double of the value of the temperature in degrees Celsius.
         */
        virtual double read_temp() = 0;
        virtual ~TempSensor() = default;
};