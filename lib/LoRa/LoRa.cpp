#include "LoRa.h"

// source: https://stackoverflow.com/questions/25890784/computing-length-of-a-c-string-at-compile-time-is-this-really-a-constexpr
static int constexpr length(const char* str) {
    return *str ? 1 + length(str + 1) : 0;
}

LoRa::LoRa(LoRaSerial* lora_serial, const uint16_t timeout, const uint8_t num_retries, const uint32_t freq, const DataRate data_rate, const Bandwidth bandwidth, const CodeRate code_rate, const uint8_t tx_power, const LNA lna, const LowDrOpt low_dr_opt) : lora_serial_(lora_serial), timeout_(timeout), num_retries_(num_retries), freq_(freq), data_rate_(data_rate), bandwidth_(bandwidth), code_rate_(code_rate), tx_power_(tx_power), lna_(lna), low_dr_opt_(low_dr_opt) {}

bool LoRa::begin() {
    this->lora_serial_->flush_buffer();
    if (!this->reset()) {
        return false;
    }
    if (!this->configure()) {
        return false;
    }
    this->state_ = LoRaState::IDLE;
    return true;
}

bool LoRa::reset() {
    const char command_buffer[] = AT_RESET "\r\n";
    const char expected_buffer[] = "AT? to list all available functions\r\n";
    return this->send_command_(command_buffer, strlen(command_buffer), expected_buffer, strlen(expected_buffer));
}

static char* ultoa_internal(char* buffer, size_t buf_size, unsigned long val) {
    char* ptr = &buffer[buf_size - 1];
    *ptr = '\0';
    do {
        *--ptr = '0' + (val % 10);
        val /= 10;
    } while (val > 0);
    return ptr;
}

static char* utoa_internal(char* buffer, size_t buf_size, unsigned int val) {
    char* ptr = &buffer[buf_size - 1];
    *ptr = '\0';
    do {
        *--ptr = '0' + (val % 10);
        val /= 10;
    } while (val > 0);
    return ptr;
}

// copy string and return new end pointer (different to normal `strcpy` sort of like `strcat`)
static char* strcpy_internal(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    return dest;
}

bool LoRa::configure() {
    char command_buffer[length(AT_CCONF) + 40];
    // snprintf(command_buffer, ARR_SIZE(command_buffer), AT_CCONF "=%lu:%u:%u:%u:4/%u:%u:%u\r\n", this->freq_, this->tx_power_, (uint8_t)this->bandwidth_, (uint8_t)this->data_rate_, (uint8_t)this->code_rate_, (uint8_t)this->lna_, (uint8_t)this->low_dr_opt_);
    char* ptr = command_buffer;
    
    // Copy the AT command prefix
    ptr = strcpy_internal(ptr, AT_CCONF "=");
    
    // Convert frequency
    char freq_buf[12];  // Max 10 digits for 32-bit + null
    char* freq_ptr = ultoa_internal(freq_buf, sizeof(freq_buf), this->freq_);
    ptr = strcpy_internal(ptr, freq_ptr);
    *ptr++ = ':';
    
    // Convert and append other parameters
    char num_buf[6];  // Max 5 digits + null
    
    ptr = strcpy_internal(ptr, utoa_internal(num_buf, sizeof(num_buf), this->tx_power_));
    *ptr++ = ':';
    
    ptr = strcpy_internal(ptr, utoa_internal(num_buf, sizeof(num_buf), (uint8_t)this->bandwidth_));
    *ptr++ = ':';
    
    ptr = strcpy_internal(ptr, utoa_internal(num_buf, sizeof(num_buf), (uint8_t)this->data_rate_));
    *ptr++ = ':';
    
    *ptr++ = '4';
    *ptr++ = '/';
    
    ptr = strcpy_internal(ptr, utoa_internal(num_buf, sizeof(num_buf), (uint8_t)this->code_rate_));
    *ptr++ = ':';
    
    ptr = strcpy_internal(ptr, utoa_internal(num_buf, sizeof(num_buf), (uint8_t)this->lna_));
    *ptr++ = ':';
    
    ptr = strcpy_internal(ptr, utoa_internal(num_buf, sizeof(num_buf), (uint8_t)this->low_dr_opt_));
    
    // Add line ending
    *ptr++ = '\r';
    *ptr++ = '\n';
    *ptr = '\0';
    
    const char expected_buffer[] = "\r\nOK\r\n";
    return this->send_command_(command_buffer, ptr - command_buffer, expected_buffer, strlen(expected_buffer));
}

bool LoRa::send_command_(const char* command, const size_t command_length, const char* expected, const size_t expected_length) {
    this->lora_serial_->flush_buffer();
    for (uint8_t i = 0; i < this->num_retries_; ++i) {
        watchdog.refresh();
        this->lora_serial_->write(command, command_length);
        if (!this->lora_serial_->expected(expected, expected_length, serial_buffer_, LORA_SERIAL_BUFFER_SIZE, this->timeout_)) {
            continue;
        }
        return true;
    }
    return false;
}

static void byte_to_hex(uint8_t byte, char* out) {
    static const char hex_chars[] = "0123456789abcdef";
    out[0] = hex_chars[byte >> 4];
    out[1] = hex_chars[byte & 0x0F];
}

bool LoRa::send(const char* data, const size_t data_size) {
    if (data_size > LORA_MAX_SIZE) {
        return false;
    }

    DEBUG_LORA_PRINT("LoRa current state: "); DEBUG_LORA_PRINTLN((uint8_t)this->state_);

    const LoRaState saved_state = this->state_;
    if (!this->set_state(LoRaState::TX)) {
        this->set_state(saved_state);   
        return false;
    }

    constexpr size_t buffer_size = ARR_SIZE(this->buffer_);
    constexpr size_t tx_command_size = length(AT_CTX) + 1; // `+ 1` for the `"="`
    strncpy(this->buffer_, AT_CTX "=", buffer_size);
    
    // append data in 2-byte padded hex format with no separation
    size_t data_offset = tx_command_size;
    for (size_t i = 0; i < data_size; ++i) {
        if (data_offset + 2 >= buffer_size) {
            break;
        }
        // snprintf(buffer_ + data_offset, buffer_size - data_offset, "%02x", data[i]);
        byte_to_hex(data[i], buffer_ + data_offset);
        data_offset += 2;
    }

    strncpy(this->buffer_ + data_offset, "\r\n", buffer_size - data_offset);

    this->buffer_counter_ = data_size;
    const char expected_buffer[] = "OnTxDone\r\n";
    if (!this->send_command_(this->buffer_, data_offset + 2, expected_buffer, strlen(expected_buffer))) { // the `+ 2` is for the extra `"\r\n"`
        this->set_state(saved_state);
        return false;
    }

    this->set_state(saved_state); // ignore return value, we still successfully sent
    return true;
}

bool LoRa::set_state(const LoRaState updated_state) {
    switch (updated_state) {
        case LoRaState::RX:
            return enter_rx_mode_();
        case LoRaState::TX:
            return enter_tx_mode_();
        case LoRaState::IDLE:
            return enter_idle_mode_();
        case LoRaState::SLEEP:
            return enter_idle_mode_(); // `IDLE` is already low power
        default:
            return false;
    }
}

LoRaState LoRa::get_state() {
    return this->state_;
}

bool LoRa::enter_rx_mode_() {
    if (!configure()) {
        return false;
    }

    const char command_buffer[] = AT_CRX "\r\n";
    const char expected_buffer[] = "Receiving...\r\n";
    return this->send_command_(command_buffer, strlen(command_buffer), expected_buffer, strlen(expected_buffer));
}

bool LoRa::enter_tx_mode_() {
    if (!enter_idle_mode_()) {
        return false;
    }

    return configure();
}

bool LoRa::enter_idle_mode_() {
    const char command_buffer[] = AT_COFF "\r\n";
    const char expected_buffer[] = "Idle...\r\n";
    return this->send_command_(command_buffer, strlen(command_buffer), expected_buffer, strlen(expected_buffer));
}

uint8_t hex_char_to_byte(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    // invalid
    return (uint8_t)-1;
}

bool LoRa::recv() {
    // ideally, we should already be in RX mode, so that if recv is called a previous message can be read from the serial buffer
    const LoRaState saved_state = this->state_;
    if (!this->set_state(LoRaState::RX)) {
        goto fail;
    }
    for (uint8_t i = 0; i < this->num_retries_; ++i) {
        watchdog.refresh();
        buffer_counter_ = 0;
        const char expected_buffer[] = "Recv:\r\n";
        if (!this->lora_serial_->expected(expected_buffer, strlen(expected_buffer), serial_buffer_, LORA_SERIAL_BUFFER_SIZE, this->timeout_)) {
            continue;
        }
        int c1 = this->lora_serial_->read_blocking(timeout_);
        if (c1 == -1) {
            goto fail;
        }
        int c2 = this->lora_serial_->read_blocking(timeout_);
        if (c2 == -1) {
            goto fail;
        }
        while (c1 != '\r') {
            watchdog.refresh();
            if (buffer_counter_ >= LORA_MAX_SIZE) {
                goto fail;
            }
            const uint8_t c1_byte = hex_char_to_byte(c1);
            const uint8_t c2_byte = hex_char_to_byte(c2);
            if ((c1_byte == (uint8_t)-1) || (c2_byte == (uint8_t)-1)) { // invalid hex formats
                goto fail;
            }
            this->buffer_[buffer_counter_] = (c1_byte << 4) | c2_byte;
            buffer_counter_++;
            if (this->lora_serial_->read_blocking(timeout_) != ' ') { // flush ' ' char
                goto fail;
            }
            c1 = this->lora_serial_->read_blocking(timeout_);
            if (c1 == -1) {
                goto fail;
            }
            c2 = this->lora_serial_->read_blocking(timeout_);
            if (c2 == -1) {
                goto fail;
            }
        }
        // the rx ends with `"Data end\r\nrssi = {rssi} dBm, snr = {snr} dB\r\n"`
        const char expected_buffer_2[] = "Data end\r\nrssi = ";
        if (!this->lora_serial_->expected(expected_buffer_2, strlen(expected_buffer_2), serial_buffer_, LORA_SERIAL_BUFFER_SIZE, this->timeout_)) {
            goto fail;
        }
        const char expected_buffer_3[] = " dB\r\n";
        if (!this->lora_serial_->expected(expected_buffer_3, strlen(expected_buffer_3), serial_buffer_, LORA_SERIAL_BUFFER_SIZE, this->timeout_)) {
            goto fail;
        }
        #ifdef DEBUG
        int rssi = 0, snr = 0;
        sscanf(serial_buffer_, "%d dBm, snr = %d dB\r\n", &rssi, &snr);
        #endif
        DEBUG_LORA_PRINT("Received rssi: ");
        DEBUG_LORA_PRINT(rssi);
        DEBUG_LORA_PRINT(", snr: ");
        DEBUG_LORA_PRINTLN(snr);
        this->set_state(saved_state);
        return true;
    }
    fail:
    this->set_state(saved_state);   
    return false;
}

const char* LoRa::get_buffer() {
    return this->buffer_;
}

uint8_t LoRa::get_buffer_len() {
    return this->buffer_counter_;
}