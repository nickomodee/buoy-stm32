#pragma once

/**
 * @brief Interface for reading the UV index.
 */
class UVSensor {
    public:
        virtual bool init() { return true; };
        /**
         * @brief Read the UV index.
         * 
         * @returns A double of the value of the UV index on the UV index scale.
         */
        virtual double read_uv_index() = 0;
        virtual ~UVSensor() = default;
};