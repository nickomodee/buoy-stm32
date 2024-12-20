#pragma once

#include "../FirmwareUpdater/FirmwareUpdater.h"
#include "../Debug/Debug.h"
#include "../PAL/PAL.h"
#include "../RealTimeClock/RealTimeClock.h"
#include <cstdint>

typedef bool (*DataStreamParserFirmwareInitFunc)(const uint32_t expected_size, const firmware_checksum_type expected_checksum); // static, so no need for pointer to member
typedef void (*DataStreamParserFirmwareStreamFunc)(const uint8_t* data, size_t size); // static, so no need for pointer to member
typedef bool (*DataStreamParserFirmwareFinaliseFunc)(const bool success); // static, so no need for pointer to member

enum class DataStreamParserState {
    SECOND,
    MINUTE,
    HOUR,
    DAY,
    DAYOFWEEK,
    MONTH,
    YEAR,
    FIRMWARE_SIZE,
    FIRMWARE_CHECKSUM,
    FIRMWARE
};

class DataStreamParser {
    public:
        DataStreamParser();
        static void reset();
        static void parse_data(const char* data, const size_t size, const uint32_t current_index, const uint32_t final_index);

    private:
        static bool handle_state_(bool final);

        static const DataStreamParserFirmwareInitFunc firmware_init_func_;
        static const DataStreamParserFirmwareStreamFunc firmware_stream_func_;
        static const DataStreamParserFirmwareFinaliseFunc firmware_finalise_func_;
        static constexpr size_t BUFFER_SIZE = 255;
        static uint8_t buffer_[BUFFER_SIZE];
        static size_t buffer_offset_;
        static size_t bytes_needed_;
        static DataStreamParserState state_;
        // protocol fields
        static uint8_t second_;
        static uint8_t minute_;
        static uint8_t hour_;
        static uint8_t day_;
        static uint8_t dayofweek_;
        static uint8_t month_;
        static uint16_t year_;
        static firmware_size_type expected_firmware_size_;
        static firmware_size_type current_firmware_size_;
        static firmware_checksum_type expected_firmware_checksum_;
        static firmware_checksum_type current_firmware_checksum_;
};

extern DataStreamParser data_stream_parser;