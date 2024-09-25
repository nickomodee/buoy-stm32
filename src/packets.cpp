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
    blink(5);
    PAL_SERIAL.begin(9600);
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
    // const uint8_t packet_data[] = {0x35, 0x02, 0x2e, 0x8f, 0x85, 0x47, 0x00, 0x6d, 0xa1, 0x89, 0x03, 0x57, 0x71, 0xa8, 0xcc, 0x22, 0x1e, 0xf4, 0x8a, 0x80, 0x61, 0xeb, 0x51, 0x01, 0x2b, 0xe0, 0xed, 0x32, 0x7d, 0x86, 0xaa, 0x88, 0x9e, 0xa3, 0xf4, 0x6a, 0x44, 0xe2, 0x11, 0x3e, 0x96, 0xda, 0x8a, 0x1a, 0x01, 0x1d, 0xaf, 0x88, 0x37, 0x37, 0x69, 0x35, 0xb2};
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