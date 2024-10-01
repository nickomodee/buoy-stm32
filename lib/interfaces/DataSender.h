#pragma once

#include <cstddef>

/**
 * @brief Interface for sending data
 * 
 * Could be through LoRa, TCP, etc.
 * Implements a `send()` method.
 */
class DataSender {
    public:
        /**
         * @brief Send data from the buoy
         * 
         * @param[in] data The data to be sent to the server
         * @param[in] data_size The size of the data
         * @returns A boolean to show if the data was sent successfully
         */
        virtual bool send(const char* data, size_t data_size) = 0;
        virtual ~DataSender() = default;
};