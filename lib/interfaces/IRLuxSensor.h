#pragma once

/**
 * @brief Interface for reading the infrared light intensity in lux.
 */
class IRLuxSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the infrared light intensity in lux.
         * 
         * @returns A double of the value of the infrared light intensity in lux.
         */
        virtual double read_ir_lux() = 0;
        virtual ~IRLuxSensor() = default;
};