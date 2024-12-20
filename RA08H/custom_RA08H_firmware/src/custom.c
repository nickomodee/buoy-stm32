#include "custom.h"

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
    Radio.IrqProcess();
    // uint8_t data = (uint8_t)uart_receive_data(CONFIG_DEBUG_UART); // blocking
    while (!lpuart_get_rx_status(LPUART, LPUART_SR0_RX_DONE_STATE)) {
        Radio.IrqProcess();
    }
    uint8_t data = lpuart_receive_data(LPUART);
    lpuart_clear_rx_status(LPUART, LPUART_SR0_RX_DONE_STATE);
    CMD_GetChar(&data, 1, 0);
    CMD_Process();
}