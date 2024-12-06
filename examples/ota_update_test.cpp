#include "PAL.h"
#include "LoRaSerial.h"
#include "LoRa.h"
#include "Encryption.h"
#include "BCP.h"
#include "SD.h"
#include "FirmwareUpdater.h"
#include "DataStreamParser.h"

// LoRa config
#define LORA_TIMEOUT 500
#define LORA_NUM_RETRIES 5
#define LORA_LOCAL_ADDR 101
#define LORA_TARGET_ADDR 102
#define LORA_FREQ 915000000 // 915 MHz
#define LORA_DATA_RATE DataRate::SF12
#define LORA_BANDWIDTH Bandwidth::BANDWIDTH_250_KHZ
#define LORA_CODE_RATE CodeRate::RATE_4_BY_5
#define LORA_TX_POWER 22
#define LORA_IQCONVERTED IQConverted::OFF

// BCP config
#define BCP_TIMEOUT 5000 // these should be different between the buoy and the server and ideally prime to avoid getting stuck
#define BCP_NUM_RETRIES 14 // `timeout * num_retries` should be equal between the buoy and the server

#if PLATFORM == STM32
    PAL_STM32_UART_STREAM LoRaSerial_UART_STM32(USART1); // USART2 is used for the default 'Serial'
    #define SERIAL_INTERFACE LoRaSerial_UART_STM32
#elif PLATFORM == ARDUINO
    // #define ALTSERIAL
    #ifdef ALTSERIAL
    #include "AltSoftSerial.h" // for Uno or Nano
    AltSoftSerial lora_serial_alt(8, 9);
    #define SERIAL_INTERFACE lora_serial_alt
    #else
    #define SERIAL_INTERFACE Serial1
    #endif
#else
    #error "Unsupported platform: PLATFORM must be defined as STM32 or ARDUINO. Please refer to 'PAL.h'"
#endif

LoRaSerial lora_serial(SERIAL_INTERFACE);
LoRa lora(&lora_serial, LORA_TIMEOUT, LORA_NUM_RETRIES, LORA_LOCAL_ADDR, LORA_TARGET_ADDR, LORA_FREQ, LORA_DATA_RATE, LORA_BANDWIDTH, LORA_CODE_RATE, LORA_TX_POWER, LORA_IQCONVERTED);

const uint8_t encryption_key[16] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 };
Encryption encryption(encryption_key);

FirmwareUpdater firmware_updater;
DataStreamParser data_stream_parser;

void setup() {
    PAL_SERIAL.begin(9600);
    SERIAL_INTERFACE.begin(9600);

    HAL_RTC_Init

    // PAL_SERIAL.println("Update succesful!"); // uncomment to test

    uint8_t sd_init_status;
    while (true) {
        sd_init_status = sd.begin();
        if (sd_init_status == 1) {
            break;
        }

        PAL_SERIAL.print("SD Initialisation Failed, Status: "); // TODO: when the SD is not initialised from the beginning (e.g. unplugged), it will never be initialised (e.g. if you plug it back in)
        PAL_SERIAL.println(sd_init_status);
        PAL_DELAY(1000);
    }
    PAL_SERIAL.println("SD Initialised");
}

void loop() {
    const BCPDataStreamFunc bcp_data_stream_func = [](const char* data, const size_t size, const uint32_t current_index, const uint32_t final_index) {
        data_stream_parser.parse_data(data, size, current_index, final_index);
    };
    BCP bcp_instance(BCP_TIMEOUT, BCP_NUM_RETRIES, &lora, bcp_data_stream_func, &encryption);
    const char data[] = "hello world PADDING PADDING PADDING PADDING PADDING PADDING PADDING PADDING";
    const bool success = bcp_instance.send(data, strlen(data));
    PAL_SERIAL.print("BCP Status: ");
    PAL_SERIAL.println(success);
    if (firmware_updater.check()) {
        PAL_SERIAL.println("Updating...");
        firmware_updater.update();
    }
    PAL_SERIAL.println("Not updating");

    PAL_DELAY(5000);
}