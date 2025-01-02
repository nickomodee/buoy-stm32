#include "PAL.h"
#include "LoRaSerial.h"
#include "LoRa.h"
#include "Encryption.h"
#include "BCP.h"
#include "SD.h"
#include "FirmwareUpdater.h"
#include "DataStreamParser.h"
#include "Watchdog.h"

#include "BNO085.h"
#include "BatteryVoltageReader.h"
#include <array>
#include "TempSensor.h"
#include "HumiditySensor.h"
#include "PressureSensor.h"
#include "UVSensor.h"
#include "VisibleLightSensor.h"
#include "IRLightSensor.h"

#include "BME280.h"
#include "DS18B20.h"
#include "SHT30.h"
// #include "SI1145.h"
#include "GUVAS12SD.h"
#include "Buoy.h"

// LoRa config
#define LORA_TIMEOUT 10000
#define LORA_NUM_RETRIES 1
#define LORA_FREQ 915000000 // 915 MHz
#define LORA_DATA_RATE DataRate::SF11
#define LORA_BANDWIDTH Bandwidth::BANDWIDTH_125_KHZ
#define LORA_CODE_RATE CodeRate::RATE_4_BY_5
#define LORA_TX_POWER 22
#define LORA_LNA LNA::ON
#define LORA_LOW_DR_OPT LowDrOpt::AUTO

// BCP config
#define BCP_TIMEOUT 13000 // these should be different between the buoy and the server and ideally prime to avoid getting stuck
#define BCP_NUM_RETRIES 10 // `timeout * num_retries` should be similar between the buoy and the server

PAL_STM32_UART_STREAM LoRaSerial_UART_STM32(USART1); // USART2 is used for the default 'Serial'
#define LORA_SERIAL_INTERFACE LoRaSerial_UART_STM32

LoRaSerial lora_serial(LORA_SERIAL_INTERFACE);
LoRa lora(&lora_serial, LORA_TIMEOUT, LORA_NUM_RETRIES, LORA_FREQ, LORA_DATA_RATE, LORA_BANDWIDTH, LORA_CODE_RATE, LORA_TX_POWER, LORA_LNA, LORA_LOW_DR_OPT);

const uint8_t encryption_key[16] = { 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41 };
Encryption encryption(encryption_key);

const BCPDataStreamFunc bcp_data_stream_func = [](const char* data, const size_t size, const uint32_t current_index, const uint32_t final_index) {
    data_stream_parser.parse_data(data, size, current_index, final_index);
};
BCP bcp_instance(BCP_TIMEOUT, BCP_NUM_RETRIES, &lora, bcp_data_stream_func, &encryption);

constexpr uint8_t bno085_reset_pin = 11;
BNO085 bno085(bno085_reset_pin);

BatteryVoltageReader battery_voltage_reader;

BME280 bme280;
constexpr uint8_t water_temp_pin = 9;
DS18B20 water_temp(water_temp_pin);
// constexpr uint8_t air_temp_pin = 3;
// DS18B20 air_temp(air_temp_pin);
SHT30 sht30;

GPIO_TypeDef* UV_GPIO_port = GPIOB;
constexpr uint8_t UV_GPIO_pin = 0;
constexpr uint32_t UV_GPIO_channel = ADC_CHANNEL_11;
GUVAS12SD guvas12sd(UV_GPIO_port, UV_GPIO_pin, UV_GPIO_channel);
// SI1145 si1145;

const std::array<TempSensor*, 3> temp_sensors_array = {&water_temp, &bme280, &sht30};
const std::array<HumiditySensor*, 2> humidity_sensors_array = {&bme280, &sht30};
const std::array<PressureSensor*, 1> pressure_sensors_array = {&bme280};
const std::array<UVSensor*, 1> uv_sensors_array = {&guvas12sd};
const std::array<VisibleLightSensor*, 0> visible_light_sensors_array = {};
const std::array<IRLightSensor*, 0> ir_light_sensors_array = {};
// const std::array<VisibleLightSensor*, 1> visible_light_sensors_array = {&si1145};
// const std::array<IRLightSensor*, 1> ir_light_sensors_array = {&si1145};

constexpr uint8_t buoy_init_retries = 10;
constexpr firmware_version_type buoy_firmware_version = 1.5f;

Buoy<temp_sensors_array.size(), humidity_sensors_array.size(), pressure_sensors_array.size(), uv_sensors_array.size(), visible_light_sensors_array.size(), ir_light_sensors_array.size()> buoy(buoy_init_retries, buoy_firmware_version, &bcp_instance, &bno085, &battery_voltage_reader, temp_sensors_array, humidity_sensors_array, pressure_sensors_array, uv_sensors_array, visible_light_sensors_array, ir_light_sensors_array);

void HardFault_Handler() { // unrecoverable error: invalid memory access, illegal instruction
    NVIC_SystemReset();
}

void MemManage_Handler() { // memory access violations: invalid address, non-existent memory
    NVIC_SystemReset();
}

void BusFault_Handler() { // bus error: invalid read/write to peripheral or memory region
    NVIC_SystemReset();
}

void UsageFault_Handler() { // invalid instruction usage: division by zero, unaligned memory access
    NVIC_SystemReset();
}

void enable_faults() {
    SCB->SHCSR |= (SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk); // enable all faults
}

void setup() {
    enable_faults(); // in case an error occurs we just reset instead of getting stuck

    buoy.init_rtc();
    buoy.check_and_sleep();
    watchdog.init(); // if we don't need to sleep immediately start the watchdog

    // PAL_SERIAL.begin(9600);
    LORA_SERIAL_INTERFACE.begin(9600);
    Wire.begin();
    encryption.begin();

    // PAL_SERIAL.println("Update succesful..."); // uncomment to test ota update

    buoy.init_sd();
    // PAL_SERIAL.print("SD initialisation status: ");
    // PAL_SERIAL.println(buoy.init_sd() ? "true" : "false");

    if (firmware_updater.check()) {
        // PAL_SERIAL.println("Firmware update available on setup. Updating...");
        firmware_updater.update();
    }

    buoy.init();
    // PAL_SERIAL.print("Buoy initialistation status: ");
    // PAL_SERIAL.println(buoy.init() ? "true" : "false");

    buoy.record_data();
    // PAL_SERIAL.print("Buoy recording data status: ");
    // PAL_SERIAL.println(buoy.record_data() ? "true" : "false");

    buoy.send_data();
    // PAL_SERIAL.print("Buoy sending data status: ");
    // PAL_SERIAL.println(buoy.send_data() ? "true" : "false");

    if (firmware_updater.check()) {
        // PAL_SERIAL.println("Firmware update available after sending data. Updating...");
        firmware_updater.update();
    }

    buoy.prepare_for_sleep();
}

void loop() {
    // impossible to reach here but in case pc reaches we just reset
    __disable_irq();
    HAL_NVIC_SystemReset();
}