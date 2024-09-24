#include "Encryption.h"
#include "PAL.h"

#define ARRAYSIZE(x) sizeof(x) / sizeof(x[0])

const uint8_t encryption_key[16] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 };
Encryption encryption(encryption_key);

void print_hex(const char* str, size_t str_length) {
    for (size_t i = 0; i < str_length; ++i) {
        PAL_SERIAL.print("0x");
        PAL_SERIAL.print((uint8_t)str[i], PAL_HEX);
        PAL_SERIAL.print(" ");
    }
    PAL_SERIAL.println();
}

void setup() {
    PAL_SERIAL.begin(9600);
    const uint16_t iv = encryption.generate_random_iv();
    PAL_SERIAL.print("0x"); PAL_SERIAL.println(iv, PAL_HEX);
    
    // encrypt
    const char text[] = "encrypt this";
    char encrypted_text[ARRAYSIZE(text) + AES_BLOCK_SIZE] = { 0 };
    size_t encrypted_size = encryption.encrypt(text, ARRAYSIZE(text) - 1, encrypted_text, ARRAYSIZE(encrypted_text), iv);
    print_hex(encrypted_text, encrypted_size);

    // decrypt
    char decrypted_text[ARRAYSIZE(encrypted_text)] = { 0 };
    size_t decrypted_size = encryption.decrypt(encrypted_text, encrypted_size, decrypted_text, ARRAYSIZE(decrypted_text), iv);
    print_hex(decrypted_text, decrypted_size);
    
    // custom decrypt
    const uint8_t custom_encrypted_text[] = { 36, 68, 226, 178, 237, 253, 238, 232, 189, 67, 82, 210, 214, 59, 54, 5 };
    const uint16_t custom_iv = 1337;
    const size_t custom_encrypted_size = ARRAYSIZE(custom_encrypted_text);
    char custom_decrypted_text[ARRAYSIZE(custom_encrypted_text)] = { 0 };
    size_t custom_decrypted_size = encryption.decrypt((const char*)custom_encrypted_text, custom_encrypted_size, custom_decrypted_text, ARRAYSIZE(custom_decrypted_text), custom_iv);
    print_hex(custom_decrypted_text, custom_decrypted_size);
}

void loop() {

}