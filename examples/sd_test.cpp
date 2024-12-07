#include "PAL.h"
#include "SD.h"
#include "SD_File.h"
#include <cstdbool>

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

char buffer[512] = {0};
size_t buffer_size = 0;

void setup() {
    PAL_SERIAL.begin(9600);
    PAL_DELAY(2000);

    uint8_t sd_init_status;
    while (true) {
        sd_init_status = sd.begin();
        if (sd_init_status == 1) {
            break;
        }

        PAL_SERIAL.print("SD Initialisation Failed, Status: ");
        PAL_SERIAL.println(sd_init_status);
        PAL_DELAY(1000);
    }
    PAL_SERIAL.println("SD Initialised");
}

void loop() {
    PAL_SERIAL.print("Available Space: ");
    PAL_SERIAL.print(sd.get_available_space());
    PAL_SERIAL.println(" KiB");
    PAL_SERIAL.println();

    PAL_SERIAL.print("Total Space: ");
    PAL_SERIAL.print(sd.get_total_space());
    PAL_SERIAL.println(" KiB");
    PAL_SERIAL.println();

    PAL_SERIAL.println("Files in root directory:");
    buffer_size = sd.list_dir("/", buffer, ARR_SIZE(buffer));
    PAL_SERIAL.write(buffer, buffer_size);
    PAL_SERIAL.println();

    {
        constexpr char file_path[] = "write.txt";
        SD_File file{file_path};
        PAL_SERIAL.print("Writing to \"");
        PAL_SERIAL.print(file_path);
        PAL_SERIAL.println("\":");

        PAL_SERIAL.println(file.open(FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS));
        PAL_SERIAL.print("Bytes written: ");
        size_t buffer_size = file.write("bbbbbbb\r\n", 9);
        PAL_SERIAL.println(buffer_size);
        file.close();
    }
    {
        constexpr char file_path[] = "write.txt";
        SD_File file{file_path};
        PAL_SERIAL.println(file.open(FA_READ));
        PAL_SERIAL.print("File size: ");
        PAL_SERIAL.println(file.get_size());
        PAL_SERIAL.println("Reading:");
        buffer_size = file.read(buffer, ARR_SIZE(buffer));
        PAL_SERIAL.write(buffer, buffer_size);
        PAL_SERIAL.println();
        PAL_SERIAL.println(buffer_size);
        file.close();
    }
    PAL_SERIAL.println();

    PAL_DELAY(2000);
    while (1) {};
}