#pragma once

#include "DataSender.h"
#include "Packet.h"
#include "Encryption.h"
#include "LoRa.h"
#include "Debug.h"
#include "PAL.h"
#include <cstddef>

#define BCP_SENT_HISTORY_SIZE 2
#define BCP_RECVD_HISTORY_SIZE 2

/**
 * @brief Enum class to identify what has control over transmission (i.e., the device sending packets and waiting for `ACK` packets).
 */
enum class RetransmissionController {
    BUOY,
    SERVER
};

/**
 * @brief Protocol to send and receive encrypted data over LoRa to another device.
 * 
 * An adaptation of TCP.
 * Both devices specify the number of bytes that will be sent.
 * Synchronises with the other device.
 * Sends packets of data over LoRa.
 * Receives packets of data over LoRa.
 * Handles retransmission, and duplicate packets.
 * 
 * @todo Implement a random initial index to prevent replay attacks, even if the encryption key is unknown.
 * @todo Possibly implement a rolling encryption key? Not sure if it is useful if an initial random index is used.
 */
class BCP : public DataSender {
    public:
        /**
         * @brief Construct a new BCP object.
         * 
         * @param[in] timeout The timeout in milliseconds per attempt (so one instance of `num_retries`) to receive a LoRa packet.
         * @param[in] num_retries The number of times to retry a succesful LoRa send, or the multilier for timeout when receiving.
         * @param[in] lora The LoRa object to use for sending the data.
         * @param[in] data_stream_func The function that is called when data is received. It is sent in chunks per `DATA` packet.
         * @param[in] encryption The object to use for encryption with the encryption key set.
         */
        BCP(uint16_t timeout, uint8_t num_retries, LoRa* lora, void (*data_stream_func)(const char* data, size_t data_size), Encryption* encryption);
        /**
         * @copydoc DataSender::send
         */
        bool send(const char* data, size_t data_size) override;
    private:
        uint16_t timeout; ///< Timeout duration in milliseconds for receiving LoRa packets.
        uint8_t num_retries; ///< Number of retries for sending or receiving packets.
        LoRa* lora; ///< Pointer to the LoRa object for communication.
        uint32_t current_index; ///< The current index for packets in the communication stream.
        Packet sent_packet_history[BCP_SENT_HISTORY_SIZE] {Packet(this->encryption), Packet(this->encryption)}; ///< Buffer storing sent packets for retransmission purposes.
        Packet recvd_packet_history[BCP_RECVD_HISTORY_SIZE] {Packet(this->encryption), Packet(this->encryption)}; ///< Buffer storing received packets for validation and retransmission handling.
        uint32_t total_msg_packets; ///< The total number of packets to be sent or received in the current communication.
        void (*data_stream_func)(const char* data, size_t data_size); ///< Function pointer for handling received data chunks.
        Encryption* encryption; ///< Pointer to the `Encryption` object for encrypting and decrypting packet data.
        RetransmissionController retransmission_control; ///< Control object to manage which device controls packet transmission.

        /**
         * @brief Send the packet that is at the index in the `sent_packet_history[]` buffer.
         * 
         * @param[in] history_index The index of the packet to send in the `sent_packet_history[]` buffer
         * @returns A boolean if the packet was sent successfully
         */
        bool send_packet(uint8_t history_index);
        /**
         * @brief Create a new `Packet` for sending.
         * 
         * Pushes a new `Packet` into the `sent_packet_history[]` buffer.
         * Replaces the last entry in the buffer.
         * It also increments the `current_index`. This does not allocate memory
         * 
         * @returns A pointer to a `Packet` in the `sent_packet_history[]` buffer
         */
        Packet* new_send_packet();
        /**
         * @brief Create a new `Packet` for receiving.
         * 
         * Pushes a new `Packet` into the `recvd_packet_history[]` buffer.
         * Replaces the last entry in the buffer.
         * It also increments the `current_index`. This does not allocate memory.
         * 
         * @returns A pointer to a `Packet` in the `recvd_packet_history[]` buffer.
         */
        Packet* new_recv_packet();
        /**
         * @brief Send one or more packets.
         * 
         * Wait to receive the expected packet.
         * Send the desired number of packets in order, from highest index to lowest, from the `sent_packet_history[]` buffer.
         * Waits to receive a valid packet of the expected type.
         * 
         * @param[in] num_packets The number of packets from the `sent_packet_history[]` buffer to send.
         * @param[in] expected_packet_type The expected packet type in response to the sent packets.
         * @returns A boolean if the packets were successfully sent and the response was valid and of the expected packet type.
         */
        bool send_recv(uint8_t num_packets, PacketType expected_packet_type);
        /**
         * @brief Perform the initial synchronisation handshake of the BCP stream.
         * 
         * Sends a `SYN` packet, waits for a valid `SYNACK` packet
         * Pushes the `ACK` packet to the `sent_packet_history[]` buffer without sending it, since in `send_data_desc()`, we send both this `ACK` and `DATA_DESC`.
         * In case there is a transmission failure, we send them together.
         * 
         * @returns A boolean if the receiving device successfully acknowledges our synchronisation.
         */
        bool synchronise();
        /**
         * @brief Send the description of the data to be sent to the other device (how many `DATA` packets in total that will be sent).
         * 
         * Send the `DATA_DESC` (and previous `ACK` packet for retransmission purposes).
         * Expect an `ACK` packet.
         * 
         * @param[in] data_size The size of the data array that will be sent to the other device in bytes.
         * @returns A boolean if the data description was successfully sent
         */
        bool send_data_desc(size_t data_size);
        /**
         * @brief Send the entire data buffer in chunks of packets.
         * 
         * Expects an `ACK` packet between each packet sent.
         * 
         * @returns A boolean if all of the data was successfully sent to the other device with acknowledgements.
         */
        bool send_data(const char* data, size_t data_size);
        /**
         * @brief Receive the data description packet from the other device to determine the size of the data buffer that the other server will send.
         * 
         * Expects a `DATA_DESC` packet to be received.
         * Sets the `total_msg_packets` with the data from the received data description.
         * Then we send the `ACK` packet to indicate to the other device that we have received the data description.
         * 
         * @returns A boolean if the data description was successfully received and the `ACK` packet was sent successfully.
         */
        bool recv_data_desc();
        /**
         * @brief Receive all of the expected data.
         * 
         * Expects a `DATA` packet.
         * Calls the `data_stream_func()` on the data in the buffer in each `DATA` packet received.
         * Sends an `ACK` packet in response to the other device to acknowledge that we have received the data packet.
         * If the packet received is a retransmission (`DATA_DESC` packet), then resend the `ACK`.
         * Pushes the `ACK` packet to the `sent_packet_history[]` buffer without sending it, since in `finish()`, we send both this `ACK` and `FIN`.
         * 
         * @returns A boolean if all of the data was successfully received.
         */
        bool recv_data();
        /**
         * @brief Performs the finalising handshake of the BCP stream.
         * 
         * Send the `FIN` (and previous `ACK` packet for retransmission purposes).
         * Expect an `ACK` packet.
         * Expect a subsequent `FIN` packet.
         * Responds with the final `ACK` packet.
         * 
         * @returns A boolean if the BCP stream was successfully finalised, with all expected packets sent and received successfully.
         */
        bool finish();
        /**
         * @brief Receive the next LoRa packet.
         * 
         * Validates the packet.
         * Rejects the packet if it is invalidated.
         * Does not wait until a valid packet; it only receives the next available packet.
         * 
         * @returns A boolean if a packet was successfully received and validated.
         */
        bool recv_packet();
        /**
         * @brief Checks if the last received packet is valid.
         * 
         * Checks if the checksum matches the expected checksum.
         * Checks if the packet index is expected (if the packet is in order).
         * However, it also checks if it is a retransmission packet by comparing the last 2 received packets.
         * It is only a retransmission if the IV is different, or else it is a duplicate.
         * If the IV is the same, we should not treat it as a retransmission for security reasons; a replay attack could DOS the buoy by flooding it with the same packet, which would reset the control flow indefinitely.
         * Sets the current index to the correct value if it is a retransmission.
         * 
         * @returns A boolean if the packet is valid.
         */
        bool validate_recvd_packet();
        /**
         * @brief Removes the last packet from the `recvd_packet_history[]` buffer.
         * 
         * Decrements the `current_index`.
         */
        void reject_recvd_packet();
        /**
         * @brief Set the retransmission control object.
         * 
         * Retransmission control identifies which device has control over transmission (i.e., the device sending packets and waiting for `ACK` packets).
         * 
         * @param[in] controller The device that should be in control over transmission, which will be set as the controller.
         */
        void set_retransmission_control(RetransmissionController controller);
};