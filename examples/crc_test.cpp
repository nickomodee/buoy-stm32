#include "CRC.h"
#include "PAL.h"

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

void setup() {
    PAL_SERIAL.begin(9600);
    PAL_SERIAL.println('2');

    constexpr const char data[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    crc16.reset();
    const uint16_t current_crc16 = crc16.update(data, 10);

    crc32.reset();
    const uint32_t current_crc32 = crc32.update(data, 10);
   
    crc16.reset(current_crc16); // test when interrupted
    const uint16_t calculated_crc16 = crc16.update(data + 10, strlen(data) - 10);
    PAL_SERIAL.print("Calculated CRC 16-bit: ");
    PAL_SERIAL.println(calculated_crc16);

    crc32.reset(current_crc32); // test when interrupted
    const uint32_t calculated_crc32 = crc32.update(data + 10, strlen(data) - 10);
    PAL_SERIAL.print("Calculated CRC 32-bit: ");
    PAL_SERIAL.println(calculated_crc32);
}

void loop() {}