#pragma once

#include <cstdint>
#include <cstring>

class alignas(2) Half {
    public:
        uint16_t data;

        Half();
        explicit Half(float value);
        operator float() const;
        Half& operator=(float value);

    private:
        static uint16_t float_to_half_(float value);
        float half_to_float() const;
};