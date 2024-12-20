#include "DataStreamParser.h"

const DataStreamParserFirmwareInitFunc DataStreamParser::firmware_init_func_ = &FirmwareUpdater::initialise_firmware;
const DataStreamParserFirmwareStreamFunc DataStreamParser::firmware_stream_func_ = &FirmwareUpdater::firmware_stream;
const DataStreamParserFirmwareFinaliseFunc DataStreamParser::firmware_finalise_func_ = &FirmwareUpdater::finish_firmware;
uint8_t DataStreamParser::buffer_[BUFFER_SIZE] = {0};
size_t DataStreamParser::buffer_offset_ = 0;
size_t DataStreamParser::bytes_needed_ = sizeof(second_);
DataStreamParserState DataStreamParser::state_ = DataStreamParserState::SECOND;
// protocol fields
uint8_t DataStreamParser::second_ = 0;
uint8_t DataStreamParser::minute_ = 0;
uint8_t DataStreamParser::hour_ = 0;
uint8_t DataStreamParser::day_ = 0;
uint8_t DataStreamParser::dayofweek_ = 0;
uint8_t DataStreamParser::month_ = 0;
uint16_t DataStreamParser::year_ = 0;
firmware_size_type DataStreamParser::expected_firmware_size_ = 0;
firmware_size_type DataStreamParser::current_firmware_size_ = 0;
firmware_checksum_type DataStreamParser::expected_firmware_checksum_ = 0;
firmware_checksum_type DataStreamParser::current_firmware_checksum_ = 0;

DataStreamParser::DataStreamParser() {
    reset();
}

void DataStreamParser::reset() {
    memset(buffer_, 0, DataStreamParser::BUFFER_SIZE);
    buffer_offset_ = 0;
    bytes_needed_ = sizeof(second_);
    second_ = 0;
    minute_ = 0;
    hour_ = 0;
    day_ = 0;
    dayofweek_ = 0;
    month_ = 0;
    year_ = 0;
    expected_firmware_size_ = 0;
    current_firmware_size_ = 0;
    expected_firmware_checksum_ = 0;
    current_firmware_checksum_ = 0;
    state_ = DataStreamParserState::SECOND;
}

void DataStreamParser::parse_data(const char* data, size_t size, const uint32_t current_index, const uint32_t final_index) {
    if (current_index == 0) {
        reset(); // in case the BCP transmission failed before `reset()` was called with `finish`
    }

    DEBUG_DATASTREAMPARSER_PRINT("Parsing data. Data size: ");
    DEBUG_DATASTREAMPARSER_PRINT(size);
    DEBUG_DATASTREAMPARSER_PRINT(", Current index: ");
    DEBUG_DATASTREAMPARSER_PRINT(current_index);
    DEBUG_DATASTREAMPARSER_PRINT(", Final index: ");
    DEBUG_DATASTREAMPARSER_PRINTLN(final_index);

    if (current_index > final_index) {
        DEBUG_DATASTREAMPARSER_PRINTLN("Invalid indices: current index is greater than final index. Exiting parse.");
        return;
    }

    size_t processed = 0;
    while (processed < size) {
        const size_t to_copy = PAL_MIN(size - processed, bytes_needed_ - buffer_offset_);
        DEBUG_DATASTREAMPARSER_PRINT("Copying ");
        DEBUG_DATASTREAMPARSER_PRINT(to_copy);
        DEBUG_DATASTREAMPARSER_PRINTLN(" bytes to buffer.");
        
        memcpy(buffer_ + buffer_offset_, (const uint8_t*)(data + processed), to_copy);
        buffer_offset_ += to_copy;
        processed += to_copy;

        if (buffer_offset_ == bytes_needed_) {
            DEBUG_DATASTREAMPARSER_PRINTLN("Buffer full. Handling current state.");
            if (handle_state_((processed == size) && (current_index == final_index))) {
                return; // if we are finished don't continue
            }
        }
    }

    if (current_index == final_index) {
        DEBUG_DATASTREAMPARSER_PRINTLN("Final index reached. Finalising firmware.");
        firmware_finalise_func_(false);
        reset();
    }
}

bool DataStreamParser::handle_state_(bool finish) {
    DEBUG_DATASTREAMPARSER_PRINTLN("Handling parser state...");
    if (buffer_offset_ != bytes_needed_) {
        DEBUG_DATASTREAMPARSER_PRINTLN("Buffer offset does not match bytes needed. Exiting state handler.");
        return false;
    }

    switch (state_) {
        case DataStreamParserState::SECOND:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing SECOND state...");
            second_ = buffer_[0];
            DEBUG_DATASTREAMPARSER_PRINT("Second: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(second_);
            state_ = DataStreamParserState::MINUTE;
            bytes_needed_ = sizeof(minute_);
            break;

        case DataStreamParserState::MINUTE:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing MINUTE state...");
            minute_ = buffer_[0];
            DEBUG_DATASTREAMPARSER_PRINT("Minute: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(minute_);
            state_ = DataStreamParserState::HOUR;
            bytes_needed_ = sizeof(hour_);
            break;

        case DataStreamParserState::HOUR:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing HOUR state...");
            hour_ = buffer_[0];
            DEBUG_DATASTREAMPARSER_PRINT("Hour: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(hour_);
            state_ = DataStreamParserState::DAY;
            bytes_needed_ = sizeof(day_);
            break;

        case DataStreamParserState::DAY:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing DAY state...");
            day_ = buffer_[0];
            DEBUG_DATASTREAMPARSER_PRINT("Day: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(day_);
            state_ = DataStreamParserState::DAYOFWEEK;
            bytes_needed_ = sizeof(dayofweek_);
            break;

        case DataStreamParserState::DAYOFWEEK:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing DAYOFWEEK state...");
            dayofweek_ = buffer_[0];
            DEBUG_DATASTREAMPARSER_PRINT("Day of week: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(dayofweek_);
            state_ = DataStreamParserState::MONTH;
            bytes_needed_ = sizeof(month_);
            break;

        case DataStreamParserState::MONTH:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing MONTH state...");
            month_ = buffer_[0];
            DEBUG_DATASTREAMPARSER_PRINT("Month: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(month_);
            state_ = DataStreamParserState::YEAR;
            bytes_needed_ = sizeof(year_);
            break;

        case DataStreamParserState::YEAR:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing YEAR state...");
            year_ = ((uint16_t)buffer_[0]) | ((uint16_t)buffer_[1] << 8); // little endian format
            DEBUG_DATASTREAMPARSER_PRINT("Year: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(year_);
            rtc.set(second_, minute_, hour_, day_, dayofweek_, month_, year_);
            state_ = DataStreamParserState::FIRMWARE_SIZE;
            bytes_needed_ = sizeof(expected_firmware_size_);
            break;

        case DataStreamParserState::FIRMWARE_SIZE:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing FIRMWARE_SIZE state...");
            expected_firmware_size_ = 0;
            for (size_t i = 0; i < buffer_offset_; i++) {
                expected_firmware_size_ |= (uint32_t)buffer_[i] << (i * 8);
            }
            DEBUG_DATASTREAMPARSER_PRINT("Expected firmware size: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(expected_firmware_size_);
            state_ = DataStreamParserState::FIRMWARE_CHECKSUM;
            bytes_needed_ = sizeof(firmware_checksum_type);
            break;

        case DataStreamParserState::FIRMWARE_CHECKSUM:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing FIRMWARE_CHECKSUM state...");
            expected_firmware_checksum_ = 0;
            for (size_t i = 0; i < buffer_offset_; i++) {
                expected_firmware_checksum_ |= (uint32_t)buffer_[i] << (i * 8);
            }
            DEBUG_DATASTREAMPARSER_PRINT("Expected firmware checksum: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(expected_firmware_checksum_);
            state_ = DataStreamParserState::FIRMWARE;
            bytes_needed_ = PAL_MIN(expected_firmware_size_, DataStreamParser::BUFFER_SIZE);
            if (expected_firmware_size_ > 0) {
                firmware_init_func_(expected_firmware_size_, expected_firmware_checksum_);
            }
            break;

        case DataStreamParserState::FIRMWARE:
            DEBUG_DATASTREAMPARSER_PRINTLN("Processing FIRMWARE state...");

            crc32.reset(current_firmware_checksum_); // we need to start from the previous CRC since it can be interrupted (e.g., by BCP)
            current_firmware_checksum_ = crc32.update(buffer_, buffer_offset_);

            current_firmware_size_ += buffer_offset_;

            DEBUG_DATASTREAMPARSER_PRINT("Current firmware size: ");
            DEBUG_DATASTREAMPARSER_PRINT(current_firmware_size_);
            DEBUG_DATASTREAMPARSER_PRINT(", Current checksum: ");
            DEBUG_DATASTREAMPARSER_PRINTLN(current_firmware_checksum_);

            bytes_needed_ = PAL_MIN(expected_firmware_size_ - current_firmware_size_, DataStreamParser::BUFFER_SIZE);
            firmware_stream_func_(buffer_, buffer_offset_);

            if (current_firmware_size_ >= expected_firmware_size_) {
                DEBUG_DATASTREAMPARSER_PRINT("Current firmware size has reached expected");
                finish = true;
            }

            if (finish) {
                const bool checksum_valid = (current_firmware_checksum_ == expected_firmware_checksum_);
                DEBUG_DATASTREAMPARSER_PRINTLN("Finalising firmware...");
                DEBUG_DATASTREAMPARSER_PRINT("Checksum valid: ");
                DEBUG_DATASTREAMPARSER_PRINTLN(checksum_valid ? "true" : "false");
                firmware_finalise_func_(checksum_valid);
            }
            break;
    }

    buffer_offset_ = 0;

    if (finish) {
        DEBUG_DATASTREAMPARSER_PRINTLN("Finishing state handling. Resetting parser.");
        reset();
    }

    return finish;
}

DataStreamParser data_stream_parser;