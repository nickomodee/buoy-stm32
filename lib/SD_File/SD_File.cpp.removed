#include "SD_File.h"

__FIRMWARE SD_File::SD_File(const char* path) {
    const size_t path_length = strlen(path);
    if (path_length + 1 > MAX_PATH_SIZE) {
        return;
    }
    strncpy(path_, path, path_length);
}

__FIRMWARE SD_File::~SD_File() {
    if (open_) {
        close();
    }
}

bool __FIRMWARE SD_File::open(const uint8_t mode) {
    if (open_) {
        if (!close()) {
            return false;
        }
    }

    if (sd.open(&file_, path_, mode)) {
        open_ = true;
        mode_ = mode;
        return true;
    }

    return false;
}

bool __FIRMWARE SD_File::close() {
    if (!open_) {
        return false;
    }

    if (sd.close(&file_)) {
        open_ = false;
        mode_ = 0;
        return true;
    }

    return false;
}

uint32_t __FIRMWARE SD_File::write(const uint8_t* buffer, const size_t size) {
    if ((!open_) || ((mode_ & FA_WRITE) == 0)) {
        return 0;
    }

    return sd.write(&file_, buffer, size);
}

uint32_t __FIRMWARE SD_File::write(const char* buffer, const size_t size) {
    return write((const uint8_t*)buffer, size);
}

uint32_t __FIRMWARE SD_File::write(const char* str) {
    if ((!open_) || ((mode_ & FA_WRITE) == 0)) {
        return 0;
    }

    return sd.write(&file_, str);
}

bool __FIRMWARE SD_File::write(const char data) {
    if ((!open_) || ((mode_ & FA_WRITE) == 0)) {
        return false;
    }

    return sd.write(&file_, data);
}

bool __FIRMWARE SD_File::write(const uint8_t data) {
    return write((const char)data);
}

uint32_t __FIRMWARE SD_File::read(uint8_t* buffer, const size_t size) {
    if ((!open_) || ((mode_ & FA_READ) == 0)) {
        return 0;
    }

    return sd.read(&file_, buffer, size);
}

uint32_t __FIRMWARE SD_File::read(char* buffer, const size_t size) {
    return read((uint8_t*)buffer, size);
}

int __FIRMWARE SD_File::read() {
    if (!open_) {
        return -1;
    }
    
    return sd.read(&file_);
}

uint32_t __FIRMWARE SD_File::position() {
    if (!open_) {
        return 0;
    }

    return sd.position(&file_);
}

bool __FIRMWARE SD_File::seek(const uint32_t position) {
    if (!open_) {
        return 0;
    }

    return sd.seek(&file_, position);
}

uint32_t __FIRMWARE SD_File::get_size() {
    if (!open_) {
        return 0;
    }

    return sd.file_size(&file_);
}

bool __FIRMWARE SD_File::remove() {
    close();

    return sd.remove(path_);
}

bool __FIRMWARE SD_File::rename(const char* new_path) {
    close();

    return sd.rename(path_, new_path);
}

bool __FIRMWARE SD_File::exists() {
    return sd.exists(path_);
}