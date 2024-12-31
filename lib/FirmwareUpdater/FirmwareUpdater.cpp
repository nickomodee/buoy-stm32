#include "FirmwareUpdater.h"

bool FirmwareUpdater::new_firmware_started_ = false;
firmware_size_type FirmwareUpdater::new_firmware_expected_size_ = 0;
firmware_checksum_type FirmwareUpdater::new_firmware_expected_checksum_ = 0;
firmware_size_type FirmwareUpdater::current_update_size_ = 0;
firmware_checksum_type FirmwareUpdater::current_update_checksum_ = 0;

FirmwareUpdater::FirmwareUpdater() {}

void FirmwareUpdater::firmware_stream(const uint8_t* data, const size_t size) {
    if (!new_firmware_started_ || size == 0) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware stream ignored: update not started or size is zero.");
        return;
    }

    if (current_update_size_ > new_firmware_expected_size_) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware stream aborted: current update size exceeds expected size.");
        new_firmware_started_ = false;
        return;
    }

    {
        SD_File new_firmware_file{new_firmware_path};

        if (!new_firmware_file.open(FA_WRITE | FA_OPEN_ALWAYS)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to open new firmware file for writing.");
            new_firmware_started_ = false;
            return;
        }

        if (!new_firmware_file.seek(current_update_size_)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to seek to current update size in firmware file.");
            new_firmware_started_ = false;
            return;
        }

        crc32.reset(current_update_checksum_); // we need to start from the previous CRC since it can be interrupted (e.g., by BCP)
        current_update_checksum_ = crc32.update(data, size);

        const uint32_t written_amount = new_firmware_file.write(data, size);
        current_update_size_ += written_amount;
        if (written_amount != size) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to write all data to firmware file.");
            new_firmware_started_ = false;
            return;
        }
        new_firmware_file.close();
        DEBUG_FIRMWAREUPDATER_PRINT("Firmware stream processed: ");
        DEBUG_FIRMWAREUPDATER_PRINT(size);
        DEBUG_FIRMWAREUPDATER_PRINTLN(" bytes written.");
    }
}

bool FirmwareUpdater::finish_firmware(const bool success) {
    DEBUG_FIRMWAREUPDATER_PRINTLN("Finishing firmware update...");

    if (!new_firmware_started_ || (current_update_size_ == 0) || !success || 
        (current_update_checksum_ != new_firmware_expected_checksum_) || 
        (current_update_size_ != new_firmware_expected_size_)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware update failed validation.");
        new_firmware_started_ = false;
        return false;
    }

    new_firmware_started_ = false;

    if (sd.exists(firmware_update_path) && !sd.remove(firmware_update_path)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to remove existing firmware update file.");
        return false;
    }

    if (sd.exists(firmware_update_size_path) && !sd.remove(firmware_update_size_path)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to remove existing firmware size file.");
        return false;
    }

    if (sd.exists(firmware_update_checksum_path) && !sd.remove(firmware_update_checksum_path)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to remove existing firmware checksum file.");
        return false;
    }

    if (!sd.rename(new_firmware_path, firmware_update_path)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to rename new firmware file.");
        return false;
    }

    {
        SD_File firmware_update_size_file{firmware_update_size_path};
        if (!firmware_update_size_file.open(FA_WRITE | FA_CREATE_ALWAYS)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to create size file for updated firmware.");
            return false;
        }
        if (!firmware_update_size_file.write((const uint8_t*)&new_firmware_expected_size_, update_size_size)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to write size to size file.");
            return false;
        }
        firmware_update_size_file.close();
    }

    {
        SD_File firmware_update_checksum_file{firmware_update_checksum_path};
        if (!firmware_update_checksum_file.open(FA_WRITE | FA_CREATE_ALWAYS)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to create checksum file for updated firmware.");
            return false;
        }
        if (!firmware_update_checksum_file.write((const uint8_t*)&new_firmware_expected_checksum_, update_checksum_size)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to write checksum to checksum file.");
            return false;
        }
        firmware_update_checksum_file.close();
    }

    {
        SD_File firmware_available_file{firmware_update_available_path};
        if (!firmware_available_file.open(FA_WRITE | FA_CREATE_ALWAYS)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to create firmware available file.");
            return false;
        }
        if (!firmware_available_file.write(update_is_available_indicator_byte)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to write update available indicator.");
            return false;
        }
        firmware_available_file.close();
    }

    DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware update finished successfully.");
    return true;
}

bool FirmwareUpdater::initialise_firmware(const firmware_size_type expected_size, const firmware_checksum_type expected_checksum) {
    DEBUG_FIRMWAREUPDATER_PRINTLN("Initialising firmware update...");
    new_firmware_started_ = true;
    new_firmware_expected_size_ = expected_size;
    new_firmware_expected_checksum_ = expected_checksum;
    current_update_size_ = 0;
    current_update_checksum_ = 0;

    if (sd.exists(new_firmware_path) && !sd.remove(new_firmware_path)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to remove existing new firmware file.");
        new_firmware_started_ = false;
        return false;
    }

    DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware update initialised successfully.");
    return true;
}

bool FirmwareUpdater::check() {
    DEBUG_FIRMWAREUPDATER_PRINTLN("Starting firmware update check...");

    if (!sd.exists(firmware_update_path) || !sd.exists(firmware_update_available_path) || !sd.exists(firmware_update_checksum_path) || !sd.exists(firmware_update_size_path)) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Required firmware files are missing.");
        return false;
    }

    {
        SD_File firmware_update_available_file{firmware_update_available_path};
        if (!firmware_update_available_file.open(FA_READ)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to open firmware update available file.");
            return false;
        }

        const int update_available_indicator_byte = firmware_update_available_file.read();
        if ((update_available_indicator_byte == -1) || ((char)update_available_indicator_byte != update_is_available_indicator_byte)) {
            DEBUG_FIRMWAREUPDATER_PRINT("Invalid update indicator byte: ");
            DEBUG_FIRMWAREUPDATER_PRINTLN(update_available_indicator_byte);
            return false;
        }
        firmware_update_available_file.close();
        DEBUG_FIRMWAREUPDATER_PRINTLN("Update indicator byte is valid.");
    }

    firmware_checksum_type expected_firmware_checksum = 0;
    {
        SD_File firmware_update_checksum_file{firmware_update_checksum_path};
        if (!firmware_update_checksum_file.open(FA_READ)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to open firmware checksum file.");
            return false;
        }
        if (firmware_update_checksum_file.get_size() != update_checksum_size) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Checksum file size mismatch.");
            return false;
        }
        int checksum_byte;
        for (uint8_t i = 0; i < update_checksum_size; i++) {
            watchdog.refresh();
            checksum_byte = firmware_update_checksum_file.read();
            if (checksum_byte == -1) {
                DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to read checksum byte.");
                return false;
            }
            expected_firmware_checksum |= (uint8_t)checksum_byte << (i * 8); // little endian format
        }
        firmware_update_checksum_file.close();
        DEBUG_FIRMWAREUPDATER_PRINT("Expected firmware checksum: ");
        DEBUG_FIRMWAREUPDATER_PRINTLN(expected_firmware_checksum);
    }

    firmware_size_type expected_firmware_size = 0;
    {
        SD_File firmware_update_size_file{firmware_update_size_path};
        if (!firmware_update_size_file.open(FA_READ)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to open firmware size file.");
            return false;
        }
        if (firmware_update_size_file.get_size() != update_size_size) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Checksum file size mismatch.");
            return false;
        }
        int size_byte;
        for (uint8_t i = 0; i < update_size_size; i++) {
            watchdog.refresh();
            size_byte = firmware_update_size_file.read();
            if (size_byte == -1) {
                DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to read size byte.");
                return false;
            }
            expected_firmware_size |= (uint8_t)size_byte << (i * 8); // little endian format
        }
        firmware_update_size_file.close();
        DEBUG_FIRMWAREUPDATER_PRINT("Expected firmware size: ");
        DEBUG_FIRMWAREUPDATER_PRINTLN(expected_firmware_size);
    }

    {
        SD_File firmware_file{firmware_update_path};
        if (!firmware_file.open(FA_READ)) {
            DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to open firmware file.");
            return false;
        }
        const uint32_t firmware_size = firmware_file.get_size(); // don't use `firmware_size_type` as it may overflow causing incorrect comparison
        if (firmware_size != expected_firmware_size) {
            DEBUG_FIRMWAREUPDATER_PRINT("Firmware size mismatch. Expected: ");
            DEBUG_FIRMWAREUPDATER_PRINT(expected_firmware_size);
            DEBUG_FIRMWAREUPDATER_PRINT(", Found: ");
            DEBUG_FIRMWAREUPDATER_PRINTLN(firmware_size);
            return false;
        }

        crc32.reset();
        int firmware_byte;
        for (firmware_size_type i = 0; i < firmware_size; i++) {
            watchdog.refresh();
            firmware_byte = firmware_file.read();
            if (firmware_byte == -1) {
                DEBUG_FIRMWAREUPDATER_PRINTLN("Failed to read firmware byte.");
                return false;
            }
            crc32.update((uint8_t)firmware_byte);
        }
        const firmware_checksum_type firmware_checksum = crc32.get_crc();

        if (firmware_checksum != expected_firmware_checksum) {
            DEBUG_FIRMWAREUPDATER_PRINT("Firmware checksum mismatch. Expected: ");
            DEBUG_FIRMWAREUPDATER_PRINT(expected_firmware_checksum);
            DEBUG_FIRMWAREUPDATER_PRINT(", Found: ");
            DEBUG_FIRMWAREUPDATER_PRINTLN(firmware_checksum);
            return false;
        }
        firmware_file.close();
        DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware checksum is valid. Update check passed.");
    }

    return true;
}

void FirmwareUpdater::update() {
    if (!check()) {
        DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware update check failed.");
        return;
    }

    begin_(); // will always return `false` because if it is `true`, it will automatically reset

    DEBUG_FIRMWAREUPDATER_PRINTLN("Firmware update failed.");
}

FirmwareUpdater firmware_updater;