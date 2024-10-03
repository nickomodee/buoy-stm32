#pragma once

/**
 * @brief Interface for reading humidity.
 */
class HumiditySensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the humidity.
         * 
         * @returns A double of the value of the humidity in %RH.
         */
        virtual double read_humidity() = 0;
        virtual ~HumiditySensor() = default;
};