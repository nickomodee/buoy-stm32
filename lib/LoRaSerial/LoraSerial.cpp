#include "LoRaSerial.h"

// Wait until there is data and read from device
int LoRaSerial::read_blocking(uint16_t timeout) {
    unsigned long start_time = PAL_MILLISECONDS();
    while (!this->available()) {
        if (PAL_MILLISECONDS() - start_time >= timeout) {
            return -1;
        }
    }
    return this->read();
}

bool LoRaSerial::expected(const char* target, size_t target_length, char* result, size_t result_size, uint16_t timeout) {
    result[0] = '\0';
    unsigned long start_time = 0;
    if (timeout) {
        // set stop time to current time plus timeout
        start_time = PAL_MILLISECONDS();
    }

    size_t result_length = 0;
    // keep reading until result ends with specified text or timeout is reached
    while ((result_length < target_length) || (strcmp(&result[result_length - target_length], target) != 0)) {
        // prevent buffer overflow
        if (result_length >= result_size - 1) { // ` - 1` for the null terminator
            return false;
        }

        // check if timeout reached
        if ((this->available() == 0) && (timeout) && (PAL_MILLISECONDS() - start_time >= timeout)) { // `this->available() == 0` so that we don't "cut-off" in the middle of receiving data
            return false;
        }

        // check if byte available to read (so that we don't block)
        if (this->available()) {
            char c = this->read();
            char cToStr[2];
            cToStr[0] = c;
            cToStr[1] = '\0';
            strcat(result, cToStr);
            result_length++;
        }
    }
    return true;
}

void LoRaSerial::flush_buffer() {
    while (this->available() > 0) {
        this->read();
    }
}