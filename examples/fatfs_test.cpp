#include "PAL.h"
#include "fatfs.h"
#include "SD_conf.h"

SPI_HandleTypeDef hspi;

void init_spi() {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);

    /*Configure GPIO pin : SD_CS_Pin */
    GPIO_InitStruct.Pin = SD_CS_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();

    /**SPI1 GPIO Configuration
    PA5     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA7     ------> SPI1_MOSI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hspi.Instance = SPI1;
    hspi.Init.Mode = SPI_MODE_MASTER;
    hspi.Init.Direction = SPI_DIRECTION_2LINES;
    hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi.Init.NSS = SPI_NSS_SOFT;
    hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
    hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi.Init.CRCPolynomial = 7;
    hspi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi) != HAL_OK) {
        Error_Handler();
    }
}

void setup() {
    PAL_SERIAL.begin(9600);

    init_spi();
    MX_FATFS_Init();

    PAL_SERIAL.println("\r\n~ SD card demo by kiwih ~\r\n");

    PAL_DELAY(1000); //a short delay is important to let the SD card settle

    //some variables for FatFs
    FATFS FatFs; 	//Fatfs handle
    FIL fil; 		//File handle
    FRESULT fres; //Result after operations

    //Open the file system
    fres = f_mount(&FatFs, "", 1); //1=mount now
    if (fres != FR_OK) {
        PAL_SERIAL.print("f_mount error (");
        PAL_SERIAL.print(fres);
        PAL_SERIAL.println(")");
        while(1);
    }

    //Let's get some statistics from the SD card
    DWORD free_clusters, free_sectors, total_sectors;

    FATFS* getFreeFs;

    fres = f_getfree("", &free_clusters, &getFreeFs);
    if (fres != FR_OK) {
        PAL_SERIAL.print("f_getfree error (");
        PAL_SERIAL.print(fres);
        PAL_SERIAL.println(")");
        while(1);
    }

    //Formula comes from ChaN's documentation
    total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
    free_sectors = free_clusters * getFreeFs->csize;

    PAL_SERIAL.println("SD card stats:");
    PAL_SERIAL.print(total_sectors / 2);
    PAL_SERIAL.println(" KiB total drive space.");
    PAL_SERIAL.print(free_sectors / 2);
    PAL_SERIAL.println(" KiB available.");

    //Now let's try to open file "write.txt"
    fres = f_open(&fil, "write.txt", FA_READ);
    if (fres != FR_OK) {
        PAL_SERIAL.print("f_open error (");
        PAL_SERIAL.print(fres);
        PAL_SERIAL.println(")");
        while(1);
    }
    PAL_SERIAL.println("I was able to open 'write.txt' for reading!");

    //Read 30 bytes from "test.txt" on the SD card
    BYTE readBuf[30];
    
    size_t read_amount = 0;

    fres = f_read(&fil, readBuf, sizeof(readBuf) / sizeof(readBuf[0]), &read_amount);
    if(fres == FR_OK) {
        PAL_SERIAL.print("Read string (");
        PAL_SERIAL.print(read_amount);
        PAL_SERIAL.print(") from 'write.txt' contents: ");
        PAL_SERIAL.write((const char*)readBuf, read_amount);
        PAL_SERIAL.println();
    } else {
        PAL_SERIAL.print("f_read error (");
        PAL_SERIAL.print(fres);
        PAL_SERIAL.println(")");
    }

    //Be a tidy kiwi - don't forget to close your file!
    f_close(&fil);

    // //Now let's try and write a file "write.txt"
    // fres = f_open(&fil, "write.txt", FA_WRITE | FA_OPEN_ALWAYS);
    // if(fres == FR_OK) {
    //     PAL_SERIAL.println("I was able to open 'write.txt' for writing");
    // } else {
    //     PAL_SERIAL.print("f_open error (");
    //     PAL_SERIAL.print(fres);
    //     PAL_SERIAL.println(")");
    // }

    // //Copy in a string
    // strncpy((char*)readBuf, "ddddddddddd", 19);
    // UINT bytes_written;
    // fres = f_write(&fil, readBuf, 19, &bytes_written);
    // if(fres == FR_OK) {
    //     PAL_SERIAL.print("Wrote ");
    //     PAL_SERIAL.print(bytes_written);
    //     PAL_SERIAL.println(" bytes to 'write.txt'!");
    // } else {
    //     PAL_SERIAL.print("f_write error (");
    //     PAL_SERIAL.print(fres);
    //     PAL_SERIAL.println(")");
    // }

    //Be a tidy kiwi - don't forget to close your file!
    f_close(&fil);

    //We're done, so de-mount the drive
    f_mount(NULL, "", 0);
}

void loop() {}