#include "Encryption.h"

Encryption::Encryption(const uint8_t key[AES_BLOCK_SIZE]) {
    this->set_encryption_key(key);
    this->setPadMode(paddingMode::CMS); // standard PKCS7 inverted padding (padding occurs after the data)
    PAL_RANDOMSEED_INIT_ENTROPY();
}

size_t Encryption::encrypt(const char* input, size_t input_length, char* output, size_t output_length, uint16_t iv) {
    calc_size_n_pad(input_length);
    if (output_length < (size_t)get_size()) {
        return 0;
    }
    byte plain_p[get_size()];
    padPlaintext(input, plain_p);

    uint8_t iv_block[AES_BLOCK_SIZE];
    this->set_iv_block(iv, iv_block);

    int blocks = get_size() / N_BLOCK;
    set_key(key, AES_BLOCK_SIZE * 8) ;
    cbc_encrypt(plain_p, reinterpret_cast<uint8_t*>(output), blocks, iv_block);
    return get_size();
}

size_t Encryption::decrypt(const char* input, size_t input_length, char* output, size_t output_length, uint16_t iv) {
    if (input_length % AES_BLOCK_SIZE != 0) {
        DEBUG_ENCRYPTION_PRINTLN(F("ERROR! Input size must be a multiple of the AES block size"));
        return 0;
    }

    if (output_length < input_length) {
        DEBUG_ENCRYPTION_PRINTLN(F("ERROR! Output buffer is too small"));
        return 0;
    }

    uint8_t iv_block[AES_BLOCK_SIZE];
    this->set_iv_block(iv, iv_block);

    set_size(input_length);
    int blocks = input_length / N_BLOCK;
    set_key(key, AES_BLOCK_SIZE * 8);
    cbc_decrypt (reinterpret_cast<const uint8_t*>(input), reinterpret_cast<uint8_t*>(output), blocks, iv_block);
    return get_unpadded_len(reinterpret_cast<uint8_t*>(output), input_length);
}

void Encryption::set_encryption_key(const uint8_t key[AES_BLOCK_SIZE]) {
    memcpy(this->key, key, AES_BLOCK_SIZE);
}

uint16_t Encryption::generate_random_iv() {
    return PAL_RANDOM(0, UINT16_MAX);
}

void Encryption::set_iv_block(uint16_t iv, uint8_t iv_block[AES_BLOCK_SIZE]) {
    memset(iv_block, 0, AES_BLOCK_SIZE);
    iv_block[0] = iv & 0xFF;
    iv_block[1] = iv >> 8;
}