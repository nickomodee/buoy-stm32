#include "PAL.h"
#include "Encryption.h"
#include "Packet.h"

#define ARRAYSIZE(x) sizeof(x) / sizeof(x[0])

static void print_hex(const char* str, size_t str_length) {
    for (size_t i = 0; i < str_length; ++i) {
        PAL_SERIAL.print("0x");
        PAL_SERIAL.print((uint8_t)str[i], PAL_HEX);
        PAL_SERIAL.print(" ");
    }
    PAL_SERIAL.println();
}

const uint8_t encryption_key[16] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 };
Encryption encryption(encryption_key);

void setup() {
    PAL_SERIAL.begin(9600);
    encryption.begin();
    
    const char data[] = "package this";
    const uint8_t data_size = strlen(data);
    const PacketType packet_type = PacketType::DATA;
    const uint32_t packet_index = 1001;
    Packet packet(&encryption, packet_type, packet_index, data_size, data);
    packet.generate_raw();
    const char* packet_str = packet.get_packet();
    const uint8_t packet_size = packet.get_packet_size();
    print_hex(packet_str, packet_size);
    PAL_SERIAL.println((uint8_t)packet.get_type());
    PAL_SERIAL.println(packet.get_index());
    PAL_SERIAL.println(packet.get_message_checksum());
    PAL_SERIAL.println(packet.generate_message_checksum());
    PAL_SERIAL.println(packet.validate_message_checksum());
    PAL_SERIAL.println(packet.get_data_size());
    PAL_SERIAL.println(packet.get_packet_checksum());
    PAL_SERIAL.println(packet.generate_packet_checksum());
    PAL_SERIAL.println(packet.validate_packet_checksum());
    PAL_SERIAL.println(packet.get_packet_size());
    PAL_SERIAL.println(packet.get_iv());
    PAL_SERIAL.println((uint8_t)packet.get_illegal_char_replacement());

    Packet decoded_packet(&encryption);
    decoded_packet.set_packet_size(packet_size);
    decoded_packet.set_packet(packet_str);
    // const uint8_t packet_data[] = {0x26, 0x0, 0xF1, 0xEE, 0xB9, 0xEE, 0xBA, 0xAA, 0xBE, 0xC8, 0x77, 0xFF, 0xFB, 0xFA, 0x93, 0x9F, 0x33, 0x9, 0xDE, 0x82, 0x9C, 0xE6, 0x1A, 0x6B, 0x8B, 0x9B, 0x5, 0xA9, 0xAF, 0x85, 0x2E, 0x82, 0x83, 0xD9, 0x5A, 0xDF, 0x8F, 0x2E};
    // decoded_packet.set_packet_size(ARRAYSIZE(packet_data));
    // decoded_packet.set_packet((const char*)packet_data);
    if (!decoded_packet.from_raw()) {
        PAL_SERIAL.println("ERROR: Decryption failed!");
    }
    const char* decoded_data = decoded_packet.get_data();
    const uint8_t decoded_data_size = decoded_packet.get_data_size();
    print_hex(decoded_data, decoded_data_size);
    PAL_SERIAL.println((uint8_t)decoded_packet.get_type());
    PAL_SERIAL.println(decoded_packet.get_index());
    PAL_SERIAL.println(decoded_packet.get_message_checksum());
    PAL_SERIAL.println(decoded_packet.generate_message_checksum());
    PAL_SERIAL.println(decoded_packet.validate_message_checksum());
    PAL_SERIAL.println(decoded_packet.get_data_size());
    PAL_SERIAL.println(decoded_packet.get_packet_checksum());
    PAL_SERIAL.println(decoded_packet.generate_packet_checksum());
    PAL_SERIAL.println(decoded_packet.validate_packet_checksum());
    PAL_SERIAL.println(decoded_packet.get_packet_size());
    PAL_SERIAL.println(decoded_packet.get_iv());
    PAL_SERIAL.println((uint8_t)decoded_packet.get_illegal_char_replacement());
    print_hex(decoded_packet.get_packet(), decoded_packet.get_packet_size());
}

void loop() {

}