#pragma once

#include "../FirmwareUpdater/FirmwareUpdater.h"
#include "../Debug/Debug.h"
#include "../PAL/PAL.h"
#include <cstdint>

typedef bool (*DataStreamParserFirmwareInitFunc)(const uint32_t expected_size, const firmware_checksum_type expected_checksum); // static, so no need for pointer to member
typedef void (*DataStreamParserFirmwareStreamFunc)(const uint8_t* data, size_t size); // static, so no need for pointer to member
typedef bool (*DataStreamParserFirmwareFinaliseFunc)(const bool success); // static, so no need for pointer to member

enum class DataStreamParserState {
    FIRMWARE_SIZE,
    FIRMWARE_CHECKSUM,
    FIRMWARE
};

class DataStreamParser {
    public:
        DataStreamParser();
        void reset();
        void parse_data(const char* data, const size_t size, const uint32_t current_index, const uint32_t final_index);

    private:
        bool handle_state_(bool final);

        const DataStreamParserFirmwareInitFunc firmware_init_func_ = &FirmwareUpdater::initialise_firmware;
        const DataStreamParserFirmwareStreamFunc firmware_stream_func_ = &FirmwareUpdater::firmware_stream;
        const DataStreamParserFirmwareFinaliseFunc firmware_finalise_func_ = &FirmwareUpdater::finish_firmware;
        static constexpr size_t BUFFER_SIZE = 8;
        uint8_t buffer_[BUFFER_SIZE] = {0};
        size_t buffer_offset_ = 0;
        size_t bytes_needed_ = sizeof(expected_firmware_size_);
        DataStreamParserState state_ = DataStreamParserState::FIRMWARE_SIZE;
        // protocol fields
        firmware_size_type expected_firmware_size_ = 0;
        firmware_size_type current_firmware_size_ = 0;
        firmware_checksum_type expected_firmware_checksum_ = 0;
        firmware_checksum_type current_firmware_checksum_ = 0;
};