#include "custom.h"

gpio_t*  g_test_gpiox = GPIOA;
uint32_t g_test_pin   = GPIO_PIN_11;

static void setup();
static void loop();

void custom_main() {
    setup();

    while (1) {
        loop();
    }
}

static void setup() {
    CMD_Init(NULL);

    printf("ATtention command interface\r\n");
    printf("AT? to list all available functions\r\n");
}

static void loop() {
    uint8_t data = (uint8_t)uart_receive_data(CONFIG_DEBUG_UART); // blocking
    CMD_GetChar(&data, 1, 0);
    CMD_Process();
}