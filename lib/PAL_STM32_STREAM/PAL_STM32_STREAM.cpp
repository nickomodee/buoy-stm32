#include "PAL_STM32_STREAM.h"

PAL_STM32_STREAM_BUFFER::PAL_STM32_STREAM_BUFFER() : head_(0), tail_(0) {}

void PAL_STM32_STREAM_BUFFER::init() {
    head_ = 0;
    tail_ = 0;
}

bool PAL_STM32_STREAM_BUFFER::isEmpty() const {
    return (head_ == tail_);
}

bool PAL_STM32_STREAM_BUFFER::isFull() const {
    uint16_t next_head = (head_ + 1) % BUFFER_SIZE;
    return (next_head == tail_);
}

void PAL_STM32_STREAM_BUFFER::put(const uint8_t data) {
    uint16_t next_head = (head_ + 1) % BUFFER_SIZE;

    buffer_[head_] = data;
    head_ = next_head;
}

bool PAL_STM32_STREAM_BUFFER::get(uint8_t &data) {
    if (isEmpty()) {
        return false;
    }

    data = buffer_[tail_];
    tail_ = (tail_ + 1) % BUFFER_SIZE;
    return true;
}

bool PAL_STM32_STREAM_BUFFER::peek(uint8_t &data) {
    if (isEmpty()) {
        return false;
    }

    data = buffer_[tail_];
    return true;
}

uint16_t PAL_STM32_STREAM_BUFFER::getCount() const {
    if (head_ >= tail_) {
        return head_ - tail_;
    } else {
        return BUFFER_SIZE - tail_ + head_;
    }
}

// PAL_STM32_STREAM --------------------------------------------------------------------------

int PAL_STM32_STREAM::available() {
    return this->UART_buffer_.getCount();
}

int PAL_STM32_STREAM::read() {
    uint8_t data;
    if (!this->UART_buffer_.get(data)) { // Empty, return -1, as the Arduino `Stream` class does
        return -1;
    }
    return data;
}

int PAL_STM32_STREAM::peek() {
    uint8_t data;
    if (!this->UART_buffer_.peek(data)) { // Empty, return -1, as the Arduino `Stream` class does
        return -1;
    }
    return data;
}

void PAL_STM32_STREAM::put_buffer() {
    const bool wasIrqEnabled = ~(__get_PRIMASK() & 1);
    if (wasIrqEnabled) {
        __disable_irq();
    }
    this->UART_buffer_.put(this->rx_byte_);
    if (wasIrqEnabled) {
        __enable_irq();
    }
}

const volatile uint8_t* PAL_STM32_STREAM::get_rx_byte_ptr() const {
    return &this->rx_byte_;
}

size_t PAL_STM32_STREAM::write(const char* str) {
    return write((const uint8_t*)str, strlen(str));
}

size_t PAL_STM32_STREAM::write(const uint8_t* buffer, const size_t size) {
    size_t written = 0;

    for (size_t i = 0; i < size; i++) {
        written += write(buffer[i]);
    }

    return written;
}

size_t PAL_STM32_STREAM::write(const char* buffer, const size_t size) {
    return write((const uint8_t*)buffer, size);
}