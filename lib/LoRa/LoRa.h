#pragma once

#include "../LoRaSerial/LoRaSerial.h"
#include "../Debug/Debug.h"

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])
#define LORA_SERIAL_BUFFER_SIZE 640
#define LORA_MAX_SIZE 255 // max packet size

/**
 * @brief Enumeration for LoRa data rate.
 * 
 * Represents different spreading factors.
 * Each value corresponds to a specific rate.
 */
enum class DataRate {
    SF7 = 7,
    SF8 = 8,
    SF9 = 9,
    SF10 = 10,
    SF11 = 11,
    SF12 = 12
};

/**
 * @brief Enumeration for LoRa bandwidth.
 * 
 * Specifies available bandwidth options in kHz.
 */
enum class Bandwidth {
    BANDWIDTH_7_81_KHZ = 0,
    BANDWIDTH_15_63_KHZ = 1,
    BANDWIDTH_31_25_KHZ = 2,
    BANDWIDTH_62_5_KHZ = 3,
    BANDWIDTH_125_KHZ = 4,
    BANDWIDTH_250_KHZ = 5,
    BANDWIDTH_500_KHZ = 6
};

/**
 * @brief Enumeration for LoRa coding rate.
 * 
 * Defines different error correction ratios.
 */
enum class CodeRate {
    RATE_4_BY_5 = 5,
    RATE_4_BY_6 = 6,
    RATE_4_BY_7 = 7,
    RATE_4_BY_8 = 8
};

/**
 * @brief Enumeration for LNA (Low Noise Amplifier).
 * 
 * Enables or disables LNA.
 */
enum class LNA {
    OFF = 0,
    ON = 1
};

/**
 * @brief Enumeration for LowDrOpt (Low Data Rate Optimisation).
 * 
 * Enables or disables LowDrOpt.
 */
enum class LowDrOpt {
    OFF = 0,
    ON = 1,
    AUTO = 2
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
    SLEEP // same as idle but backwards compatible with RA-08H
};

#define AT_CCONF "AT+CCONF"
#define AT_CTX "AT+CTX"
#define AT_CRX "AT+CRX"
#define AT_COFF "AT+COFF"
#define AT_RESET "ATZ"

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
     * @param[in] freq Operating frequency in Hz.
     * @param[in] data_rate Data rate (spreading factor).
     * @param[in] bandwidth Bandwidth setting.
     * @param[in] code_rate Error correction coding rate.
     * @param[in] tx_power Transmission power level.
     * @param[in] lna LNA setting.
     * @param[in] low_dr_opt LowDrOpt setting.
     */
    LoRa(LoRaSerial* lora_serial,
         const uint16_t timeout,
         const uint8_t num_retries,
         const uint32_t freq,
         const DataRate data_rate,
         const Bandwidth bandwidth,
         const CodeRate code_rate,
         const uint8_t tx_power,
         const LNA lna,
         const LowDrOpt low_dr_opt);

    /**
     * @brief Begins LoRa communication by initialising settings.
     * 
     * A boolean if initialisation is successful.
     */

    bool begin();

    /**
     * @brief Performs a software reset of the LoRa device.
     * 
     * A boolean if the reset is successful.
     */
    bool reset();
    
    /**
     * @brief Configure the radio settings for LoRa communication.
     * 
     * A boolean if the configuration is successful.
     */
    bool configure();

    /**
     * @brief Sends data to the target device.
     * 
     * @param[in] data Pointer to the data to be sent.
     * @param[in] data_size Size of the data to send.
     * @returns A boolean if data is sent successfully.
     */
    bool send(const char* data, const size_t data_size);

    /**
     * @brief Sets the LoRa device state.
     * 
     * @param[in] state Desired state of the LoRa device.
     * @returns A boolean if the state is successfully set.
     */
    bool set_state(const LoRaState state);

    /**
     * @brief Retrieves the current state of the LoRa device.
     * 
     * @returns The current state of the LoRa device.
     */
    LoRaState get_state();

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
    uint8_t get_buffer_len();

private:
    LoRaSerial* lora_serial_; ///< Pointer to the LoRaSerial object for communication.
    char serial_buffer_[LORA_SERIAL_BUFFER_SIZE]; ///< Buffer for serial communication data.
    uint16_t timeout_; ///< Timeout duration in milliseconds for operations.
    uint8_t num_retries_; ///< Number of retries for failed commands.
    uint32_t freq_; ///< Operating frequency in Hz.
    DataRate data_rate_; ///< LoRa data rate (spreading factor).
    Bandwidth bandwidth_; ///< LoRa bandwidth setting.
    CodeRate code_rate_; ///< LoRa coding rate (error correction).
    uint8_t tx_power_; ///< Transmission power level in dBm (0-22).
    LNA lna_; ///< LNA setting.
    LowDrOpt low_dr_opt_; ///< LowDrOpt setting.
    LoRaState state_ = LoRaState::IDLE; ///< Current state of the LoRa device.
    char buffer_[ARR_SIZE(AT_CTX) + 1 + (LORA_MAX_SIZE * 2) + 2 + 1]; ///< Buffer for sending and receiving data, which allows for the `"\r\n\0"` sequence on top of `{AT_CTX}=` + `LORA_MAX_SIZE`. `* 2` because we are sending data in hex format
    uint8_t buffer_counter_; ///< The size of the buffer for transmitted and receive data in `buffer[]`.

    /**
     * @brief Sends a command to the LoRa module and checks for the expected response.
     * 
     * @param[in] command The command to send.
     * @param[in] command_length The length of the command string.
     * @param[in] expected The expected response from the module.
     * @param[in] expected_length The length of the expected response.
     * @returns A boolean if the command is successful.
     */
    bool send_command_(const char* command,
                      const size_t command_length,
                      const char* expected,
                      const size_t expected_length);

    bool enter_rx_mode_();
    bool enter_tx_mode_();
    bool enter_idle_mode_();
};