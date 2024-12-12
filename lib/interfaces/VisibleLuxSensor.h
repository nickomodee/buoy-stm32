#pragma once

/**
 * @brief Interface for reading the visible light intensity in lux.
 */
class VisibleLuxSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the visible light intensity in lux.
         * 
         * @returns A float of the value of the visible light intensity in lux.
         */
        virtual float read_visible_lux() = 0;
        virtual ~VisibleLuxSensor() = default;
};