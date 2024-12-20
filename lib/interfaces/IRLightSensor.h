#pragma once

/**
 * @brief Interface for reading the infrared light.
 */
class IRLightSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the infrared light.
         * 
         * @returns A uint16_t of the value of the infrared light.
         */
        virtual uint16_t read_ir() = 0;
        virtual ~IRLightSensor() = default;
};