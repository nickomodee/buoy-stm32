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
         * @returns A float of the value of the infrared light intensity in lux.
         */
        virtual float read_ir_lux() = 0;
        virtual ~IRLuxSensor() = default;
};