#pragma once

/**
 * @brief Interface for reading pressure.
 */
class PressureSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the pressure.
         * 
         * @returns A float of the value of the pressure in Pascals.
         */
        virtual float read_pressure() = 0;
        virtual ~PressureSensor() = default;
};