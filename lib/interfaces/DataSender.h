#pragma once

#include <cstddef>

/**
 * @brief Interface for sending data.
 * 
 * Could be through LoRa, TCP, etc.
 * Implements a `send()` method.
 */
class DataSender {
    public:
        /**
         * @brief Send data from the buoy.
         * 
         * @param[in] data The data to be sent to the server.
         * @param[in] data_size The size of the data (not including the data in the file).
         * @param[in] file_path The name of the file with the rest of the data (or `nullptr` if there is no extra data).
         * @returns A boolean to show if the data was sent successfully.
         */
        virtual bool send(const char* data, size_t data_size, const char* file_path = nullptr) = 0;
        virtual ~DataSender() = default;
};