#pragma once

#include <cstdint>

/**
 * @brief Interface for reading visible light.
 */
class VisibleLightSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the visible light value.
         * 
         * @returns A uint16_t of the value of the visible light.
         */
        virtual uint16_t read_visible() = 0;
        virtual ~VisibleLightSensor() = default;
};