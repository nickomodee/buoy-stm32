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
         * @returns A float of the value of the humidity in %RH.
         */
        virtual float read_humidity() = 0;
        virtual ~HumiditySensor() = default;
};