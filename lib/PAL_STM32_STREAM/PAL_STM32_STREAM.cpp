#include "PAL_STM32_STREAM.h"

PAL_STM32_STREAM_BUFFER::PAL_STM32_STREAM_BUFFER(const uint16_t buffer_size) : BUFFER_SIZE(buffer_size), head_(0), tail_(0), count_(0) {
    this->buffer_ = new uint8_t[BUFFER_SIZE];
}

PAL_STM32_STREAM_BUFFER::~PAL_STM32_STREAM_BUFFER() {
    delete[] this->buffer_;
}

void PAL_STM32_STREAM_BUFFER::init() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
}

bool PAL_STM32_STREAM_BUFFER::is_empty() const {
    // return (head_ == tail_);
    return count_ == 0;
}

bool PAL_STM32_STREAM_BUFFER::is_full() const {
    // uint16_t next_head = (head_ + 1) % BUFFER_SIZE;
    // return (next_head == tail_);
    return count_ == BUFFER_SIZE;
}

void PAL_STM32_STREAM_BUFFER::empty() {
    head_ = tail_;
    count_ = 0;
}

void PAL_STM32_STREAM_BUFFER::reset() {
    head_ = 0;
    tail_ = 0;
    count_ = 0;
}

void PAL_STM32_STREAM_BUFFER::put(const uint8_t data) {
    if (is_full()) {
        tail_ = (tail_ + 1) % BUFFER_SIZE; // buffer is full so we need to overwrite the oldest data by moving the tail forward by one to disicard the oldest element
        count_--; // we removed the oldest element so decrement
    }

    uint16_t next_head = (head_ + 1) % BUFFER_SIZE;

    buffer_[head_] = data;
    head_ = next_head;
    count_++;
}

bool PAL_STM32_STREAM_BUFFER::get(uint8_t &data) {
    if (is_empty()) {
        return false;
    }

    data = buffer_[tail_];
    tail_ = (tail_ + 1) % BUFFER_SIZE;
    count_--;
    return true;
}

bool PAL_STM32_STREAM_BUFFER::peek(uint8_t &data) {
    if (is_empty()) {
        return false;
    }

    data = buffer_[tail_];
    return true;
}

const uint8_t* PAL_STM32_STREAM_BUFFER::get_read_ptr() {
    return const_cast<uint8_t*>(this->buffer_) + tail_;
}

const uint8_t* PAL_STM32_STREAM_BUFFER::get_write_ptr() {
    return const_cast<uint8_t*>(this->buffer_) + head_;
}

uint16_t PAL_STM32_STREAM_BUFFER::remaining_capacity() const {
    return BUFFER_SIZE - count_;
}

uint16_t PAL_STM32_STREAM_BUFFER::get_count() const {
    return count_;
}

void PAL_STM32_STREAM_BUFFER::set_count(const uint16_t count) {
    head_ = (head_ + count) % BUFFER_SIZE;
    count_ = count > BUFFER_SIZE ? BUFFER_SIZE : count;
}

// PAL_STM32_STREAM --------------------------------------------------------------------------

int PAL_STM32_STREAM::available() {
    return this->rx_buffer_.get_count();
}

int PAL_STM32_STREAM::read() {
    uint8_t data;
    if (!this->rx_buffer_.get(data)) { // Empty, return -1, as the Arduino `Stream` class does
        return -1;
    }
    return data;
}

int PAL_STM32_STREAM::peek() {
    uint8_t data;
    if (!this->rx_buffer_.peek(data)) { // Empty, return -1, as the Arduino `Stream` class does
        return -1;
    }
    return data;
}

void PAL_STM32_STREAM::put_buffer(const uint8_t data) {
    STM32_INTERRUPT_GUARD interrupt_guard;
    this->rx_buffer_.put(data);
}

const volatile uint8_t* PAL_STM32_STREAM::get_rx_byte_ptr() const {
    return &this->rx_byte_;
}

uint8_t PAL_STM32_STREAM::get_rx_byte() const {
    return this->rx_byte_;
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