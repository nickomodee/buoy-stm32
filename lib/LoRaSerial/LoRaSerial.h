#pragma once

#include <cstddef>
#include <inttypes.h>
#include <cstring>
#include <cstdint>
#include "PAL.h"

/**
 * @brief A wrapper class for serial communication with LoRa devices using a PAL_STREAM object.
 *
 * Provides a convenient interface for serial communication with LoRa devices.
 * Wraps the `PAL_STREAM` object (e.g., `HardwareSerial`, `SoftwareSerial`), enabling basic read, write, and buffer flush operations.
 * Offers methods for blocking reads and reading until or checking for expected serial responses.
 *
 */
class LoRaSerial {
    public:
        /**
         * @brief Construct a new LoRa Serial object.
         * 
         * @param[in] serial Reference to the `PAL_STREAM` object for serial communication.
         */
        LoRaSerial(PAL_STREAM& serial) : _serial(serial) {}

        /**
         * @brief Reads a byte from the serial PAL_STREAM while blocking until a timeout is reached.
         * 
         * @param[in] timeout Maximum time in milliseconds to wait for data.
         * @returns The byte read from the PAL_STREAM, or -1 if the timeout is reached.
         */
        int read_blocking(uint16_t timeout);

        /**
         * @brief Reads data from the serial PAL_STREAM until the expected target string is found, the timeout is reached, or the `result[]` buffer is filled.
         * 
         * Reads incoming data and appends it to the `result[]` buffer until the expected target string is found at the end of the buffer or a timeout occurs.
         * If the buffer is full or the timeout is reached without matching the target, the function returns false.
         * The `result[]` buffer contains the bytes read from the serial PAL_STREAM, including the target at the end, if it was successfully found.
         * 
         * @param[in] target The expected byte sequence to match.
         * @param[in] target_length The length of the expected sequence.
         * @param[out] result Buffer to store the received data.
         * @param[in] result_size Size of the result buffer.
         * @param[in] timeout Maximum time in milliseconds to wait for the expected sequence.
         * @returns A boolean if the target sequence successfully matches in the time limit, or also fails if the `result[]` buffer is filled and the target isn't found.
         */
        bool expected(const char* target, size_t target_length, char* result, size_t result_size, uint16_t timeout);

        /**
         * @brief Flushes the serial buffer by clearing all available data.
         */
        void flush_buffer();

        // PAL_STREAM wrapper methods
        /**
         * @brief Returns the number of bytes available for reading from the PAL_STREAM.
         * 
         * @returns The number of bytes available.
         */
        int available() {
            return _serial.available();
        }

        /**
         * @brief Reads a byte from the serial PAL_STREAM.
         * 
         * @returns The byte read from the PAL_STREAM, or -1 if no data is available.
         */
        int read() {
            return _serial.read();
        }

        /**
         * @brief Writes a single byte to the serial PAL_STREAM.
         * 
         * @param[in] data The byte to write.
         * @returns The number of bytes written (should be 1).
         */
        size_t write(uint8_t data) {
            return _serial.write(data);
        }

        /**
         * @brief Writes a null-terminated string to the serial PAL_STREAM.
         * 
         * @param[in] str The string to write.
         * @returns The number of bytes written.
         */
        size_t write(const char* str) {
            return _serial.write(str);
        }

        /**
         * @brief Writes a buffer of data to the serial PAL_STREAM.
         * 
         * @param[in] buffer Pointer to the data buffer.
         * @param[in] size Size of the buffer.
         * @returns The number of bytes written.
         */
        size_t write(const uint8_t* buffer, size_t size) {
            return _serial.write(buffer, size);
        }

        /**
         * @brief Writes a buffer of characters to the serial PAL_STREAM.
         * 
         * @param[in] buffer Pointer to the character buffer.
         * @param[in] size Size of the buffer.
         * @returns The number of bytes written.
         */
        size_t write(const char* buffer, size_t size) {
            return _serial.write((const uint8_t*)buffer, size);
        }

    private:
        PAL_STREAM& _serial; ///< Reference to the PAL_STREAM object that handles the actual communication.
};