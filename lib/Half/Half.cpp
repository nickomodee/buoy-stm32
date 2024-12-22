#include "Half.h"

Half::Half() : data(0) {}

Half::Half(float value) : data(float_to_half_(value)) {}

Half::operator float() const {
    return half_to_float();
}

Half& Half::operator=(float value) {
    data = float_to_half_(value);
    return *this;
}

uint16_t Half::float_to_half_(float value) {
    uint32_t f;
    memcpy(&f, &value, sizeof(value));
    uint32_t sign = (f >> 31) & 0x1;
    uint32_t exp = (f >> 23) & 0xff;
    uint32_t frac = f & 0x7fffff;

    if (exp == 0xff) {
        return (sign << 15) | 0x7c00 | (frac ? (frac >> 13) : 0);
    }
    
    int newExp = exp - 127 + 15;
    if (newExp <= 0) return (sign << 15);
    if (newExp >= 0x1f) return (sign << 15) | 0x7c00;

    return (sign << 15) | (newExp << 10) | (frac >> 13);
}

float Half::half_to_float() const {
    uint32_t sign = (data >> 15) & 0x1;
    uint32_t exp = (data >> 10) & 0x1f;
    uint32_t frac = data & 0x3ff;

    if (exp == 0x1f) {
        exp = 0xff;
        frac = frac ? (frac << 13) : 0;
    } else if (exp != 0) {
        exp = exp - 15 + 127;
        frac <<= 13;
    } else if (frac != 0) {
        frac <<= 13;
    }

    uint32_t f = (sign << 31) | (exp << 23) | frac;
    float result;
    memcpy(&result, &f, sizeof(f));
}