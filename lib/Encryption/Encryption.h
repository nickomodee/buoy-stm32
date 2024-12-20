#pragma once
#include "../AES/AES.h"
#include "../Debug/Debug.h"
#include "../PAL/PAL.h"
#include <cstdint>

#define AES_BLOCK_SIZE 16 // 128 bits

/**
 * @brief Class for AES encryption and decryption for BCP.
 * 
 * Utilises CBC mode padding: standard PKCS7 inverted padding (padding occurs after the data).
 * Utilises a simple initialisation vector (IV) for encryption as only 2 bytes (to keep the packet size small).
 * Supports 128 bits and 256 bits, but this cannot be changed at runtime.
 * BCP uses 128 bits since the data transmitted per packet has to be quite small, so we use 48 bytes.
 */
class Encryption : public AES {
    public:
        /**
         * @brief Construct a new Encryption object.
         * 
         * @param[in] key A buffer of bytes representing the AES encryption key, which is a valid key size.
         */
        Encryption(const uint8_t key[AES_BLOCK_SIZE]);
        /**
         * @brief Initialise the random seed
         * 
         */
        void begin();
        /**
         * @brief Encrypts the input data using AES encryption in CBC mode.
         * 
         * Encrypts the given input data, applying padding if necessary, and writes the encrypted output to the provided output buffer.
         * The output buffer must be large enough to hold the encrypted data.
         * Uses AES encryption with a simple initialisation vector (IV).
         *
         * @param[in] input A pointer to the input data buffer that contains the plaintext to be encrypted.
         * @param[in] input_length The length of the input data in bytes.
         * @param[out] output A pointer to the output buffer where the encrypted data will be stored.
         * @param[in] output_length The length of the output buffer in bytes, which must be at least as large as the padded size of the input.
         * @param[in] iv The initialisation vector (IV) used for encryption, provided as a 16-bit integer (the IV is stored in the first two bytes of the AES block, with the rest of the block zeroed out.)
         * @returns The size of the encrypted output in bytes, or 0 if the output buffer is too small.
         * 
         * 
         */
        size_t encrypt(const char* input, size_t input_length, char* output, size_t output_length, uint16_t iv);
        /**
         * @brief Decrypts the input data using AES decryption in CBC mode.
         * 
         * Decrypts the given input data and writes the decrypted output to the provided output buffer.
         * The input data length must be a multiple of the AES block size.
         * The output buffer must be large enough to hold the decrypted data, which is at least the size of the input.
         * Uses AES decryption with a simple initialisation vector (IV).
         * 
         * @param[in] input A pointer to the input data buffer that contains the ciphertext to be decrypted.
         * @param[in] input_length The length of the input data in bytes, which must be a multiple of the AES block size.
         * @param[out] output A pointer to the output buffer where the decrypted data will be stored.
         * @param[in] output_length The length of the output buffer in bytes, which must be at least as large as the input length.
         * @param[in] iv The initialisation vector (IV) used for decryption, provided as a 16-bit integer (the IV is stored in the first two bytes of the AES block, with the rest of the block zeroed out.)
         * @returns The size of the decrypted output in bytes, or 0 if the input length is not a multiple of the AES block size or if the output buffer is too small.
         */
        size_t decrypt(const char* input, size_t input_length, char* output, size_t output_length, uint16_t iv);
        /**
         * @brief Generates a random initialises vector (IV) to be used for AES encryption or decryption.
         * 
         * Generates a 16-bit integer, as we want to use the least amount of bytes needed, as the packets for BCP over LoRa have a limited size.
         * The generated IV is from the range 0 (inclusive) to UINT16_MAX (exclusive), for a total of 65535 possibilities.
         * 
         * @todo Make this a 32-bit integer to essentially guarantee that there aren't any collisions to mitigate replay attacks and cryptographic analysis.
         * @return A 16-bit integer.
         */
        static uint16_t generate_random_iv();
    private:
        uint8_t key[AES_BLOCK_SIZE]; ///< A buffer of bytes containing the AES encryption key, equal to the AES block size, and cannot be changed after instantiation.

        /**
         * @brief Sets the encryption key for AES encryption and decryption.
         * 
         * Copies the provided encryption key into the internal key storage.
         * The key size must match the AES block size.
         * 
         * @param[in] key A buffer containing the AES encryption key, which must be equal to the AES block size.
         */
        void set_encryption_key(const uint8_t key[AES_BLOCK_SIZE]);
        /**
         * @brief Sets the initialisation vector (IV) for AES operations.
         * 
         * Initialises the IV storage with the provided 16-bit value.
         * The IV is stored in the first two bytes of the AES block, with the rest of the block zeroed out.
         * 
         * @todo Make this a 32-bit integer to essentially guarantee that there aren't any collisions to mitigate replay attacks and cryptographic analysis.
         * @param[in] iv A 16-bit integer used to initialise the first two bytes of the AES initialisation vector (IV).
         * @param[out] iv_block A buffer to store the initialisation vector, which must be the size of the AES block.
         */
        static void set_iv_block(uint16_t iv, uint8_t iv_block[AES_BLOCK_SIZE]);
};