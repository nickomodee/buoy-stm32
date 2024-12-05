#pragma once

#include "../LoRaSerial/LoRaSerial.h"
#include "../Debug/Debug.h"

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])
#define LORA_SERIAL_BUFFER_SIZE 512
#define LORA_MAX_SIZE 61

/**
 * @brief Enumeration for LoRa data rate.
 * 
 * Represents different spreading factors.
 * Each value corresponds to a specific rate.
 */
enum class DataRate {
    SF12 = 0,
    SF11 = 1,
    SF10 = 2,
    SF9 = 3,
    SF8 = 4,
    SF7 = 5,
    SF6 = 6,
    SF5 = 7
};

/**
 * @brief Enumeration for LoRa bandwidth.
 * 
 * Specifies available bandwidth options in kHz.
 */
enum class Bandwidth {
    BANDWIDTH_125_KHZ = 0,
    BANDWIDTH_250_KHZ = 1,
    BANDWIDTH_500_KHZ = 2,
    BANDWIDTH_62_5_KHZ = 3,
    BANDWIDTH_41_67_KHZ = 4,
    BANDWIDTH_31_25_KHZ = 5,
    BANDWIDTH_20_83_KHZ = 6,
    BANDWIDTH_15_63_KHZ = 7,
    BANDWIDTH_10_42_KHZ = 8,
    BANDWIDTH_7_81_KHZ = 9
};

/**
 * @brief Enumeration for LoRa coding rate.
 * 
 * Defines different error correction ratios.
 */
enum class CodeRate {
    RATE_4_BY_5 = 1,
    RATE_4_BY_6 = 2,
    RATE_4_BY_7 = 3,
    RATE_4_BY_8 = 4
};

/**
 * @brief Enumeration for IQ conversion.
 * 
 * Enables or disables IQ conversion.
 */
enum class IQConverted {
    OFF = 0,
    ON = 1
};

/**
 * @brief Enumeration for LoRa operational states.
 * 
 * Defines the states like RX, TX, IDLE, and SLEEP.
 */
enum class LoRaState {
    RX,
    TX,
    IDLE,
    SLEEP
};

/**
 * @brief Class for handling LoRa communication.
 * 
 * Provides methods for sending, receiving, and configuring LoRa settings.
 */
class LoRa {
public:
    /**
     * @brief Constructs a new LoRa object.
     * 
     * @param[in] lora_serial Pointer to the LoRaSerial object for communication.
     * @param[in] timeout Timeout duration in milliseconds for operations.
     * @param[in] num_retries Number of retries for failed commands.
     * @param[in] local_addr Local device address.
     * @param[in] target_addr Target device address.
     * @param[in] freq Operating frequency in Hz.
     * @param[in] data_rate Data rate (spreading factor).
     * @param[in] bandwidth Bandwidth setting.
     * @param[in] code_rate Error correction coding rate.
     * @param[in] tx_power Transmission power level.
     * @param[in] iqconverted IQ inversion setting.
     */
    LoRa(LoRaSerial* lora_serial,
         uint16_t timeout,
         uint8_t num_retries,
         uint16_t local_addr,
         uint16_t target_addr,
         uint32_t freq,
         DataRate data_rate,
         Bandwidth bandwidth,
         CodeRate code_rate,
         uint8_t tx_power,
         IQConverted iqconverted);

    /**
     * @brief Begins LoRa communication by initialising settings.
     * 
     * A boolean if initialisation is successful.
     */
    bool begin();

    /**
     * @brief Sends data to the target device.
     * 
     * @param[in] data Pointer to the data to be sent.
     * @param[in] data_size Size of the data to send.
     * @returns A boolean if data is sent successfully.
     */
    bool send(const char* data, size_t data_size);

    /**
     * @brief Sets the LoRa device state.
     * 
     * @param[in] state Desired state of the LoRa device.
     * @returns A boolean if the state is successfully set.
     */
    bool set_state(LoRaState state);

    /**
     * @brief Retrieves the current state of the LoRa device.
     * 
     * @returns The current state of the LoRa device.
     */
    LoRaState get_state();

    /**
     * @brief Wakes the LoRa device from sleep mode.
     * 
     * @returns A boolean if the device is successfully woken up.
     */
    bool wake();

    /**
     * @brief Receives data from the LoRa device.
     * 
     * @returns A boolean if data is received successfully.
     */
    bool recv();

    /**
     * @brief Gets the last received data.
     * 
     * @returns A pointer to the buffer containing the received data.
     */
    const char* get_buffer();

    /**
     * @brief Get the length of the data in the buffer.
     * 
     * @returns The length of the data in the buffer, `buffer[]`.
     */
    const uint8_t get_buffer_len();

private:
    LoRaSerial* lora_serial; ///< Pointer to the LoRaSerial object for communication.
    char serial_buffer[LORA_SERIAL_BUFFER_SIZE]; ///< Buffer for serial communication data.
    uint16_t timeout; ///< Timeout duration in milliseconds for operations.
    uint8_t num_retries; ///< Number of retries for failed commands.
    uint16_t local_addr; ///< Local device address.
    uint16_t target_addr; ///< Target device address.
    uint32_t freq; ///< Operating frequency in Hz.
    DataRate data_rate; ///< LoRa data rate (spreading factor).
    Bandwidth bandwidth; ///< LoRa bandwidth setting.
    CodeRate code_rate; ///< LoRa coding rate (error correction).
    uint8_t tx_power; ///< Transmission power level in dBm (0-22).
    IQConverted iqconverted; ///< IQ inversion setting.
    LoRaState state = LoRaState::IDLE; ///< Current state of the LoRa device.
    char buffer[LORA_MAX_SIZE + 2 + 1]; ///< Buffer for sending and receiving data, which allows for the `"\r\n\0"` sequence on top of the `LORA_MAX_SIZE`.
    uint8_t buffer_counter; ///< The size of the buffer for transmitted and receive data in `buffer[]`.

    /**
     * @brief Sends a command to the LoRa module and checks for the expected response.
     * 
     * @param[in] command The command to send.
     * @param[in] command_length The length of the command string.
     * @param[in] expected The expected response from the module.
     * @param[in] expected_length The length of the expected response.
     * @returns A boolean if the command is successful.
     */
    bool send_command(const char* command,
                      size_t command_length,
                      const char* expected,
                      size_t expected_length);

    /**
     * @brief Sets the local device address.
     * 
     * @param[in] local_addr The local address to set.
     * @returns A boolean if the local address is set successfully.
     */
    bool set_local_addr(uint16_t local_addr);

    /**
     * @brief Sets the target device address.
     * 
     * @param[in] target_addr The target address to set.
     * @returns A boolean if the target address is set successfully.
     */
    bool set_target_addr(uint16_t target_addr);
};