#include "PAL.h"
#include "LoRaSerial.h"
#include "LoRa.h"

// LoRa config
#define LORA_TIMEOUT 5000
#define LORA_NUM_RETRIES 5
#define LORA_LOCAL_ADDR 102
#define LORA_TARGET_ADDR 101
#define LORA_FREQ 920000000 // 920 MHz
#define LORA_DATA_RATE DataRate::SF12
#define LORA_BANDWIDTH Bandwidth::BANDWIDTH_250_KHZ
#define LORA_CODE_RATE CodeRate::RATE_4_BY_5
#define LORA_TX_POWER 22
#define LORA_IQCONVERTED IQConverted::OFF

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

static void print_hex(const char* str, size_t str_length) {
    for (size_t i = 0; i < str_length; ++i) {
        PAL_SERIAL.print("0x");
        PAL_SERIAL.print((uint8_t)str[i], PAL_HEX);
        PAL_SERIAL.print(" ");
    }
    PAL_SERIAL.println();
}

void setup() {
    PAL_SERIAL.begin(9600);
    SERIAL_INTERFACE.begin(9600);
    PAL_SERIAL.println(lora.begin());
    PAL_SERIAL.println(lora.set_state(LoRaState::RX));
    const char data[] = "hello world";
    PAL_SERIAL.println(lora.send(data, strlen(data)));
}

void loop() {
    if (lora.recv()) {
        PAL_SERIAL.print("Data Received: ");
        print_hex(lora.get_buffer(), lora.get_buffer_len());
        PAL_DELAY(400);
        PAL_SERIAL.println(lora.send(lora.get_buffer(), lora.get_buffer_len()));
    }
    // PAL_DELAY(1000);
}