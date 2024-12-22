#include "Packet.h"

Packet::Packet(Encryption* encryption, const PacketType type, const uint32_t index, const uint8_t data_size, const char* data) : encryption_(encryption) {
    this->set(type, index, data_size, data);
}

Packet::Packet(Encryption* encryption) : encryption_(encryption) {}

void Packet::set(const PacketType type, const uint32_t index, const uint8_t data_size, const char* data) {
    this->set_type(type);
    this->set_index(index);
    this->set_data_size(data_size); // this step must happen before setting `data`
    this->set_data(data);
}

void Packet::set_type(const PacketType type) {
    this->type = type;
}

PacketType Packet::get_type() const {
    return this->type;
}

void Packet::set_index(uint32_t index) {
    if (index == (uint32_t)-1) { // -1 is reserved for the BCP to tell it to keep the original index (if setting everything through the `set(PacketType, uint32_t, uint8_t, const char*)` method)
        return;
    }
    this->index = index;
}

uint32_t Packet::get_index() const {
    return this->index;
}

void Packet::set_message_checksum(uint16_t checksum) {
    this->message_checksum = checksum;
}

uint16_t Packet::get_message_checksum() const {
    return this->message_checksum;
}

uint16_t Packet::generate_message_checksum() {
    crc16.reset();
    crc16.update((uint8_t)this->type);
    crc16.update((uint8_t*)&this->index, sizeof(this->index)); // little endian format
    // skip checksum for calculating the checksum
    crc16.update(this->data_size);
    const uint8_t data_size = PAL_MIN(this->data_size, MAX_DATA_SIZE);
    return crc16.update(this->data, data_size);
}

bool Packet::validate_message_checksum() {
    const uint16_t real_checksum = this->generate_message_checksum();
    return real_checksum == this->message_checksum;
}

void Packet::set_data_size(uint8_t data_size) {
    this->data_size = data_size;
}

uint8_t Packet::get_data_size() const {
    return this->data_size;
}

void Packet::set_data(const char* data) {
    const uint8_t data_size = PAL_MIN(this->data_size, MAX_DATA_SIZE);
    memcpy(this->data, data, data_size);
}

const char* Packet::get_data() const {
    return this->data;
}

void Packet::set_packet_checksum(uint16_t checksum) {
    this->packet_checksum = checksum;
}

uint16_t Packet::get_packet_checksum() const {
    return this->packet_checksum;
}

uint16_t Packet::generate_packet_checksum() {
    crc16.reset();
    const uint8_t packet_size = PAL_MIN(this->packet_size, MAX_PACKET_SIZE);
    for (uint8_t i = 0; i < packet_size; ++i) {
        if ((i == PACKET_CHECKSUM_1_INDEX) || (i == PACKET_CHECKSUM_2_INDEX) || (i == PACKET_ILLEGAL_CHAR_INDEX)) { // skip checksum and illegal char replacement (since if the checksum has an illegal char, we still want to handle this) for calculating the checksum
            continue;
        }
        crc16.update(this->packet[i]);
    }
    return crc16.get_crc();
}

bool Packet::validate_packet_checksum() {
    const uint16_t real_checksum = this->generate_packet_checksum();
    return real_checksum == this->packet_checksum;
}

void Packet::set_packet_size(uint8_t packet_size) {
    this->packet_size = packet_size;
}

uint8_t Packet::get_packet_size() const {
    return (uint8_t)this->packet[PACKET_SIZE_INDEX];
}

void Packet::set_packet(const char* packet) {
    const uint8_t packet_size = PAL_MIN(this->packet_size, MAX_PACKET_SIZE);
    memcpy(this->packet, packet, packet_size);
}

const char* Packet::get_packet() const {
    return this->packet;
}

void Packet::set_iv(uint16_t iv) {
    this->iv = iv;
}

uint16_t Packet::get_iv() const {
    return this->iv;
}

void Packet::set_illegal_char_replacement(char illegal_char_replacement) {
    this->illegal_char_replacement = illegal_char_replacement;
}

char Packet::get_illegal_char_replacement() const {
    return this->illegal_char_replacement;
}

bool Packet::from_raw() {
    this->revert_illegal_char(); // Do this FIRST THING
    this->set_packet_size(this->get_packet_size());
    if (this->packet_size > MAX_PACKET_SIZE) {
        DEBUG_PACKET_PRINT("Error while parsing. Packet size: ");
        DEBUG_PACKET_PRINT(this->packet_size);
        DEBUG_PACKET_PRINT(" is larger than the maximum packet size: ");
        DEBUG_PACKET_PRINTLN(MAX_PACKET_SIZE);
    }
    this->set_illegal_char_replacement(this->packet[PACKET_ILLEGAL_CHAR_INDEX]);
    this->set_iv((uint16_t)(uint8_t)this->packet[PACKET_IV_1_INDEX] | ((uint16_t)(uint8_t)this->packet[PACKET_IV_2_INDEX] << 8));
    this->set_packet_checksum((uint16_t)(uint8_t)this->packet[PACKET_CHECKSUM_1_INDEX] | ((uint16_t)(uint8_t)this->packet[PACKET_CHECKSUM_2_INDEX] << 8));
    const uint8_t encrypted_data_size = this->packet_size - PACKET_OVERHEAD;
    char encrypted_data[encrypted_data_size];
    memcpy(encrypted_data, this->packet + PACKET_OVERHEAD, encrypted_data_size); // unfortunately, we do have to use this buffer because don't want to modify `this->packet` (if we simply use a pointer to `this->packet + PACKET_OVERHEAD`)
    char message[MAX_MESSAGE_SIZE + AES_BLOCK_SIZE]; // Even though we know the maximum size, according to the BCP, the encryption library checks the WORST case, which is when there is no padding, so it is safest to append the block size just in case to avoid buffer overflow
    const size_t message_size = this->encryption_->decrypt(encrypted_data, encrypted_data_size, message, sizeof(message) / sizeof(message[0]), this->iv);
    if ((message_size < MESSAGE_DATA_START_INDEX - 1) || (message_size != (size_t)MESSAGE_OVERHEAD + (uint8_t)message[MESSAGE_DATA_SIZE_INDEX])) {
        DEBUG_PACKET_PRINT("Error while decrypting message with invalid size: ");
        DEBUG_PACKET_PRINTLN(message_size);
        return false;
    }
    this->set_type((PacketType)message[MESSAGE_TYPE_INDEX]);
    this->set_index((uint32_t)(uint8_t)message[MESSAGE_INDEX_1_INDEX] | ((uint32_t)(uint8_t)message[MESSAGE_INDEX_2_INDEX] << 8) | ((uint32_t)(uint8_t)message[MESSAGE_INDEX_3_INDEX] << 16) | ((uint32_t)(uint8_t)message[MESSAGE_INDEX_4_INDEX] << 24));
    this->set_message_checksum((uint16_t)(uint8_t)message[MESSAGE_CHECKSUM_1_INDEX] | ((uint16_t)(uint8_t)message[MESSAGE_CHECKSUM_2_INDEX] << 8));
    this->set_data_size((uint8_t)message[MESSAGE_DATA_SIZE_INDEX]); // this SHOULD equal `message_size - MESSAGE_OVERHEAD`
    this->set_data(&message[MESSAGE_DATA_START_INDEX]);
    return true;
}

void Packet::generate_raw() {
    char message[MAX_MESSAGE_SIZE];
    message[MESSAGE_TYPE_INDEX] = (uint8_t)this->type;
    message[MESSAGE_INDEX_1_INDEX] = this->index & 0xFF;
    message[MESSAGE_INDEX_2_INDEX] = (this->index >> 8) & 0xFF;
    message[MESSAGE_INDEX_3_INDEX] = (this->index >> 16) & 0xFF;
    message[MESSAGE_INDEX_4_INDEX] = (this->index >> 24) & 0xFF;
    const uint16_t message_checksum = this->generate_message_checksum();
    this->set_message_checksum(message_checksum);
    message[MESSAGE_CHECKSUM_1_INDEX] = this->message_checksum & 0xFF;
    message[MESSAGE_CHECKSUM_2_INDEX] = this->message_checksum >> 8;
    message[MESSAGE_DATA_SIZE_INDEX] = this->data_size;
    memcpy(&message[MESSAGE_DATA_START_INDEX], this->data, PAL_MIN(this->data_size, MAX_DATA_SIZE));
    const uint16_t iv = this->encryption_->generate_random_iv();
    this->set_iv(iv);
    const size_t encrypted_size = this->encryption_->encrypt(message, MESSAGE_OVERHEAD + PAL_MIN(this->data_size, MAX_DATA_SIZE), this->packet + PACKET_OVERHEAD, MAX_PACKET_SIZE - PACKET_OVERHEAD, this->iv);
    this->set_packet_size(encrypted_size + PACKET_OVERHEAD);
    this->packet[PACKET_SIZE_INDEX] = this->packet_size;
    this->packet[PACKET_IV_1_INDEX] = this->iv & 0xFF;
    this->packet[PACKET_IV_2_INDEX] = this->iv >> 8;
    const uint16_t packet_checksum = this->generate_packet_checksum();
    this->set_packet_checksum(packet_checksum);
    this->packet[PACKET_CHECKSUM_1_INDEX] = this->packet_checksum & 0xFF;
    this->packet[PACKET_CHECKSUM_2_INDEX] = this->packet_checksum >> 8;
    // We want to set the illegal char replacement last, in case IV or checksum also contains the illegal character
    const char illegal_char_replacement = this->replace_illegal_char();
    this->set_illegal_char_replacement(illegal_char_replacement);
    this->packet[PACKET_ILLEGAL_CHAR_INDEX] = this->illegal_char_replacement;
}

#define BIT_ARRAY_SIZE 256

typedef struct {
    uint64_t data[BIT_ARRAY_SIZE / 64]; // bit array of uint64_t
} BitArray;

void set_bit(BitArray* bit_array, uint8_t position) {
    bit_array->data[position / 64] |= (1ULL << (position % 64));
}

bool get_bit(BitArray* bit_array, uint8_t position) {
    return (bit_array->data[position / 64] & (1ULL << (position % 64))) != 0;
}

char Packet::find_missing_char() {
    const uint8_t packet_size = this->get_packet_size();
    BitArray found = { 0 };
    
    for (size_t i = 0; i < packet_size; ++i) {
        if (i == PACKET_ILLEGAL_CHAR_INDEX) { // Don't consider the illegal char replacement
            continue;
        }

        set_bit(&found, this->packet[i]);
    }

    for (uint16_t i = 0; i < BIT_ARRAY_SIZE; i++) {
        if ((char)i != Packet::illegal_char && !get_bit(&found, i)) {
            return (char)i; // return a missing character (that isn't the illegal char)
        }
    }

    // bad if we reach here...
    DEBUG_PACKET_PRINTLN(F("ERROR! Missing character not generated"));
    return Packet::illegal_char;
}

char Packet::replace_illegal_char() {
    const uint8_t packet_size = this->get_packet_size();
    const char missing_char = Packet::find_missing_char();
    
    for (size_t i = 0; i < packet_size; ++i) {
        if (i == PACKET_ILLEGAL_CHAR_INDEX) { // Don't replace the illegal char replacement
            continue;
        }

        if (this->packet[i] == Packet::illegal_char) {
            this->packet[i] = missing_char;
        }
    }

    return missing_char;
}

void Packet::revert_illegal_char() {
    const uint8_t illegal_char_replacement = this->packet[PACKET_ILLEGAL_CHAR_INDEX];
    uint8_t packet_size = this->get_packet_size(); // Careful! This needs to be checked for replacing illegal char, which happens below
    packet_size = (packet_size == illegal_char_replacement) ? Packet::illegal_char : packet_size; // Check for illegal char replacement on the packet size

    for (size_t i = 0; i < packet_size; ++i) {
        if (i == PACKET_ILLEGAL_CHAR_INDEX) { // Don't replace the illegal char replacement
            continue;
        }

        if (this->packet[i] == illegal_char_replacement) {
            this->packet[i] = Packet::illegal_char;
        }
    }
}

bool Packet::operator==(const Packet &other) const {
    return (this->type == other.get_type()) && (this->index == other.get_index()) && (this->data_size == other.get_data_size()) && (memcmp(this->data, other.get_data(), this->data_size) == 0);
}

bool Packet::operator!=(const Packet &other) const {
    return !((*this) == other);
}