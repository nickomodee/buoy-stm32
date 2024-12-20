#pragma once

#include "../Encryption/Encryption.h"
#include "../PAL/PAL.h"
#include "../CRC/CRC.h"
#include "../Debug/Debug.h"
#include <cstdint>

#define MAX_PACKET_SIZE 246
#define PACKET_OVERHEAD 6 // the packet 'overhead' that isn't encrypted data
#define MAX_MESSAGE_SIZE 239
#define MESSAGE_OVERHEAD 8 // the message 'overhead' that isn't data
#define MAX_DATA_SIZE (MAX_MESSAGE_SIZE - MESSAGE_OVERHEAD) // the brackets are crucial... found out the hard way

#define MESSAGE_TYPE_INDEX 0
#define MESSAGE_INDEX_1_INDEX 1
#define MESSAGE_INDEX_2_INDEX 2
#define MESSAGE_INDEX_3_INDEX 3
#define MESSAGE_INDEX_4_INDEX 4
#define MESSAGE_CHECKSUM_1_INDEX 5
#define MESSAGE_CHECKSUM_2_INDEX 6
#define MESSAGE_DATA_SIZE_INDEX 7
#define MESSAGE_DATA_START_INDEX 8

#define PACKET_SIZE_INDEX 0
#define PACKET_ILLEGAL_CHAR_INDEX 1
#define PACKET_IV_1_INDEX 2
#define PACKET_IV_2_INDEX 3
#define PACKET_CHECKSUM_1_INDEX 4
#define PACKET_CHECKSUM_2_INDEX 5
#define PACKET_ENCRYPTED_START_INDEX 6

/**
 * @brief Enum class representing the different types of packets.
 */
enum class PacketType {
    SYN = 0,
    SYNACK = 1,
    ACK = 2,
    DATA_DESC = 3,
    DATA = 4,
    FIN = 5
};

/**
 * @brief Class representing a data packet to be transmitted over LoRa.
 * 
 * Provides the structure for sending and receiving packets.
 * Generate and validates checksums.
 */
class Packet {
    public:
        /**
         * @brief Constructs an empty Packet object ands sets the encryption object.
         * 
         * @param[in] encryption The `Encryption` object for packet data encryption and decryption.
         */
        Packet(Encryption* encryption);

        /**
         * @brief Constructs a Packet object with specified parameters.
         * 
         * @param[in] encryption The Encryption object for encryption and decryption.
         * @param[in] type The type of packet.
         * @param[in] index The packet's sequence index.
         * @param[in] data_size The size of the data in the packet.
         * @param[in] data The actual data to be transmitted.
         */
        Packet(Encryption* encryption, const PacketType type, const uint32_t index, const uint8_t data_size, const char* data);

        /**
         * @brief Sets packet parameters for transmission.
         * 
         * @param[in] type The type of packet.
         * @param[in] index The packet's sequence index.
         * @param[in] data_size The size of the data in the packet.
         * @param[in] data The actual data to be transmitted.
         */
        void set(const PacketType type, const uint32_t index, const uint8_t data_size, const char* data);

        /**
         * @brief Sets the type of packet.
         * 
         * @param[in] type The type of packet.
         */
        void set_type(const PacketType type);

        /**
         * @brief Gets the packet type.
         * 
         * @returns The PacketType of this packet.
         */
        PacketType get_type() const;

        /**
         * @brief Sets the packet index.
         * 
         * @param[in] index The sequence index of the packet.
         */
        void set_index(uint32_t index);

        /**
         * @brief Gets the packet index.
         * 
         * @returns The sequence index of the packet.
         */
        uint32_t get_index() const;

        /**
         * @brief Sets the message checksum.
         * 
         * @param[in] checksum The checksum of the message.
         */
        void set_message_checksum(uint16_t checksum);

        /**
         * @brief Gets the message checksum.
         * 
         * @returns The checksum of the message.
         */
        uint16_t get_message_checksum() const;

        /**
         * @brief Generates the message checksum.
         * 
         * @returns The generated checksum based on the packet data.
         */
        uint16_t generate_message_checksum();

        /**
         * @brief Validates the message checksum.
         * 
         * @returns A boolean if the message checksum is valid.
         */
        bool validate_message_checksum();

        /**
         * @brief Sets the data size.
         * 
         * @param[in] data_size The size of the data in the packet.
         */
        void set_data_size(uint8_t data_size);

        /**
         * @brief Gets the data size.
         * 
         * @returns The size of the data in the packet.
         */
        uint8_t get_data_size() const;

        /**
         * @brief Sets the data to be transmitted.
         * 
         * @param[in] data The data to be transmitted in the packet.
         */
        void set_data(const char* data);

        /**
         * @brief Gets the data from the packet.
         * 
         * @returns A pointer to the data in the packet.
         */
        const char* get_data() const;

        /**
         * @brief Sets the packet checksum.
         * 
         * @param[in] checksum The checksum of the packet.
         */
        void set_packet_checksum(uint16_t checksum);

        /**
         * @brief Gets the packet checksum.
         * 
         * @returns The checksum of the packet.
         */
        uint16_t get_packet_checksum() const;

        /**
         * @brief Generates the packet checksum.
         * 
         * Does not calculate based on the raw parameters, unlike `get_message_checksum()`.
         * Uses the internal `packet[]` buffer to calculate the checksum.
         * Adds up all bytes in the `packet[]` buffer, but skips the checksum.
         * 
         * @returns The generated checksum based on the packet content.
         */
        uint16_t generate_packet_checksum();

        /**
         * @brief Validates the packet checksum.
         * 
         * @returns A boolean if the packet checksum is valid.
         */
        bool validate_packet_checksum();

        /**
         * @brief Sets the size of the packet.
         * 
         * @param[in] packet_size The size of the packet.
         */
        void set_packet_size(uint8_t packet_size);

        /**
         * @brief Gets the packet size.
         * 
         * Gets the size of the packet based on the data in the packet (the byte at index 0).
         * 
         * @returns The size of the packet.
         */
        uint8_t get_packet_size() const;

        /**
         * @brief Sets the entire packet content.
         * 
         * Copies the data to the internal `packet[]` buffer.
         * 
         * @param[in] packet The data content of the packet.
         */
        void set_packet(const char* packet);

        /**
         * @brief Gets the packet content.
         * 
         * @note This should be treated as readonly and not directly modified.
         * Modification can be achieved by setting the packet parameters and regenerating the packet through `generate_packet()`.
         * @returns A pointer to the packet content.
         */
        const char* get_packet() const;

        /**
         * @brief Sets the initialisation vector (IV) for AES encryption and decryption.
         * 
         * @param[in] iv The initialisation vector used for AES encryption and decryption.
         */
        void set_iv(uint16_t iv);

        /**
         * @brief Gets the initialisation vector (IV) for AES encryption and decryption.
         * 
         * @returns The initialisation vector used for AES encryption and decryption.
         */
        uint16_t get_iv() const;

        /**
         * @brief Sets the character used to replace illegal characters in the packet.
         * 
         * @param[in] illegal_char_replacement The character used for illegal character replacement.
         */
        void set_illegal_char_replacement(char illegal_char_replacement);

        /**
         * @brief Gets a suitable character to use to replace illegal characters in the packet (multiple options available).
         * 
         * @returns A suitable replacement character for illegal characters.
         */
        char get_illegal_char_replacement() const;

        /**
         * @brief Converts raw packet data into a structured packet.
         * 
         * Uses the `packet[]` buffer for parsing.
         * The packet structure is as such:
         * 
         *  - The first byte is the size of the packet (0-53, 53 max size).
         * 
         *  - The second bytes is the byte that replaces the illegal char (`'\r'`).
         * 
         *  - The third byte is the LSB of the 16-bit initialisation vector (IV) for AES encryption and decryption.
         * 
         *  - The fourth byte is the MSB of the 16-bit initialisation vector (IV) for AES encryption and decryption.
         * 
         *  - The fifth byte is the LSB of the checksum of the packet (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *  - The sixth byte is the MSB of the checksum of the packet (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *  - The seventh up to fifty-fourth bytes (inclusive, but the packet size doesn't have to be the maximum) are the AES 128-bit encrypted message data of the packet, with the unencryped message structure below:
         * 
         *     - The first byte is the type of the packet (SYN, SYNACK, ACK, DATA_DESC, DATA, FIN, with byte values from 0 to 5)
         * 
         *     - The second byte is the LSB of the packet index.
         * 
         *     - The third byte is the SLSB of the packet index.
         * 
         *     - The fourth byte is the SMSB of the packet index.
         * 
         *     - The fifth byte is the MSB of the packet index.
         * 
         *     - The sixth byte is the LSB of the checksum of the message (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *     - The seventh byte is the MSB of the checksum of the message (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *     - The eigth byte is the size of the data, not the message (0-39, 39 max size).
         * 
         *     - The next bytes are optional: the ninth up to the forty-seventh byte is the actual data of the packet.
         * 
         * @returns A boolean if the raw packet was successfully parsed.
         */
        bool from_raw();

        /**
         * @brief Generates raw data from the current packet state.
         * 
         * The result is saved in the internal `packet[]` buffer.
         * The packet structure is as such:
         * 
         *  - The first byte is the size of the packet (0-53, 53 max size).
         * 
         *  - The second bytes is the byte that replaces the illegal char (`'\r'`).
         * 
         *  - The third byte is the LSB of the 16-bit initialisation vector (IV) for AES encryption and decryption.
         * 
         *  - The fourth byte is the MSB of the 16-bit initialisation vector (IV) for AES encryption and decryption.
         * 
         *  - The fifth byte is the LSB of the checksum of the packet (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *  - The sixth byte is the MSB of the checksum of the packet (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *  - The seventh up to fifty-fourth bytes (inclusive, but the packet size doesn't have to be the maximum) are the AES 128-bit encrypted message data of the packet, with the unencryped message structure below:
         * 
         *     - The first byte is the type of the packet (SYN, SYNACK, ACK, DATA_DESC, DATA, FIN, with byte values from 0 to 5)
         * 
         *     - The second byte is the LSB of the packet index.
         * 
         *     - The third byte is the SLSB of the packet index.
         * 
         *     - The fourth byte is the SMSB of the packet index.
         * 
         *     - The fifth byte is the MSB of the packet index.
         * 
         *     - The sixth byte is the LSB of the checksum of the message (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *     - The seventh byte is the MSB of the checksum of the message (using CRC16 of all bytes in the packet, except for the checksum bytes).
         * 
         *     - The eigth byte is the size of the data, not the message (0-39, 39 max size).
         * 
         *     - The next bytes are optional: the ninth up to the forty-seventh byte is the actual data of the packet.
         */
        void generate_raw();

        /**
         * @brief Compares two packets for equality.
         * 
         * Compares the packet type, packet index, and packet data.
         * 
         * @param[in] other The other packet to compare against.
         * @returns A boolean if the two packets are equal.
         */
        bool operator==(const Packet& other) const;

        /**
         * @brief Compares two packets for inequality.
         * 
         * Compares the packet type, packet index, and packet data.
         * 
         * @param[in] other The other packet to compare against.
         * @returns A boolean if the two packets are not equal.
         */
        bool operator!=(const Packet& other) const;

    private:
        PacketType type; ///< The type of packet.
        uint32_t index; ///< The sequence index of the packet.
        uint16_t message_checksum; ///< Checksum for the message portion of the packet.
        uint8_t data_size; ///< The size of the data in the packet.
        char data[MAX_DATA_SIZE]; ///< The data payload of the packet.
        uint16_t packet_checksum; ///< Checksum for the entire packet.
        uint16_t iv; ///< Initialisation vector for encryption.
        char illegal_char_replacement; ///< Replacement character for illegal characters in the packet.
        uint8_t packet_size; ///< The size of the packet in bytes.
        char packet[MAX_PACKET_SIZE]; ///< The full packet content, including overhead and data.
        Encryption* encryption_; ///< Pointer to the Encryption object for data encryption/decryption.
        const static char illegal_char = '\r'; ///< The character '\r' is illegal (it will trigger the LoRa device to try to send the data) and must be replaced in the packet.
        
        /**
         * @brief Find a suitable character in the internal packet to replace illegal characters
         * 
         * Returns the first character that is not present in the internal packet to use as the replacement character.
         * Skips over the character at the index of the illegal character replacement.
         * This will always works as long as the length of the string is less than or equal to 253 characters.
         * 
         * @returns The char of the character to replace the illegal character. 
         */
        char find_missing_char();

        /**
         * @brief Replaces illegal characters in the internal packet.
         * 
         * This modifies the internal packet.
         * Finds a suitable character and uses this to replace the illegal characters.
         * Skips the illegal character replacement in the internal packet.
         * 
         * @returns The illegal char replacement used to replace the illegal characters.
         */
        char replace_illegal_char();

        /**
         * @brief Reverts illegal character replacement in the internal packet.
         * 
         * This modifies the internal packet.
         * Uses the illegal char replacement in the packet data.
         */
        void revert_illegal_char();
};