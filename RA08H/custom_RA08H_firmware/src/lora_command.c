#include "lora_command.h"

static const struct ATCommand_s ATCommand[] = {
    {
        .string = AT_RESET,
        .size_string = sizeof(AT_RESET) - 1,
        #ifndef NO_HELP
        .help_string = "AT"AT_RESET" Trig a MCU reset\r\n",
        #endif /* !NO_HELP */
        .get = AT_return_error,
        .set = AT_return_error,
        .run = AT_reset,
    },
    {
        .string = AT_CCONF,
        .size_string = sizeof(AT_CCONF) - 1,
        #ifndef NO_HELP
            .help_string = "AT"AT_CCONF"=<Freq in Hz>:<Power in dBm>:<Lora Bandwidth <0 to 6>:<Lora SF>:<CodingRate 4/5, 4/6, 4/7, 4/8>:\r\n\
                <Lna>:<LowDrOpt 0:off, 1:on, 2:Auto><CR>. Configure custom RX or TX for LoRa\r\n"
            "AT+CCONF=915000000:22:4:12:4/5:1:2\r\n",
        #endif /* !NO_HELP */
        .get = AT_custom_get_config,
        .set = AT_custom_set_config,
        .run = AT_return_error,
    },

    {
        .string = AT_CTX,
        .size_string = sizeof(AT_CTX) - 1,
        #ifndef NO_HELP
            .help_string = "AT"AT_CTX"=<Packet data in 2-byte padded hex, no separation><CR>. Sends a LoRa packet\r\n",
        #endif /* !NO_HELP */
        .get = AT_return_error,
        .set = AT_custom_tx,
        .run = AT_return_error,
    },

    {
        .string = AT_CRX,
        .size_string = sizeof(AT_CRX) - 1,
        #ifndef NO_HELP
            .help_string = "AT"AT_CRX"<CR>. Starts receiving LoRa data\r\n",
        #endif /* !NO_HELP */
        .get = AT_return_error,
        .set = AT_return_error,
        .run = AT_custom_rx,
    },

    {
        .string = AT_COFF,
        .size_string = sizeof(AT_COFF) - 1,
        #ifndef NO_HELP
            .help_string = "AT"AT_COFF". Turns off on-going LoRa RX\r\n",
        #endif /* !NO_HELP */
        .get = AT_return_error,
        .set = AT_return_error,
        .run = AT_custom_off,
    },

    {
        .string = AT_CLOWPOWER,
        .size_string = sizeof(AT_CLOWPOWER) - 1,
        #ifndef NO_HELP
            .help_string = "AT"AT_CLOWPOWER". Enter low-power mode. Wakes up through interrupt over UART\r\n",
        #endif /* !NO_HELP */
        .get = AT_return_error,
        .set = AT_return_error,
        .run = AT_custom_low_power,
    }
};

static char circBuffer[CIRC_BUFF_SIZE];
static char command[CMD_SIZE];
static unsigned i = 0;
static uint32_t widx = 0;
static uint32_t ridx = 0;
static uint32_t charCount = 0;
static uint32_t circBuffOverflow = 0;

static const char *const ATError_description[] = {
    "\r\nOK\r\n",                     /* AT_OK */
    /* USER CODE BEGIN AT_BLANK_DESC */
    "",                               /* AT_BLANK */
    /* USER CODE END AT_BLANK_DESC */
    "\r\nAT_ERROR\r\n",               /* AT_ERROR */
    "\r\nAT_PARAM_ERROR\r\n",         /* AT_PARAM_ERROR */
    "\r\nAT_BUSY_ERROR\r\n",          /* AT_BUSY_ERROR */
    "\r\nAT_TEST_PARAM_OVERFLOW\r\n", /* AT_TEST_PARAM_OVERFLOW */
    "\r\nAT_NO_NETWORK_JOINED\r\n",   /* AT_NO_NET_JOINED */
    "\r\nAT_RX_ERROR\r\n",            /* AT_RX_ERROR */
    /* USER CODE BEGIN AT_TX_ERROR_DESC */
    "\r\nAT_TX_ERROR\r\n",            /* AT_TX_ERROR */
    /* USER CODE END AT_TX_ERROR_DESC */
    "\r\nAT_NO_CLASS_B_ENABLE\r\n",   /* AT_NO_CLASS_B_ENABLE */
    "\r\nAT_DUTYCYCLE_RESTRICTED\r\n", /* AT_DUTYCYCLE_RESTRICTED */
    "\r\nAT_CRYPTO_ERROR\r\n",        /* AT_CRYPTO_ERROR */
    "\r\nerror unknown\r\n",          /* AT_MAX */
};

static void parse_cmd(const char *cmd);

/**
  * @brief  Print a string corresponding to an ATEerror_t
  * @param  error_type The AT error code
  */
static void com_error(ATEerror_t error_type);

/**
  * @brief  CMD_GetChar callback from ADV_TRACE
  * @param  rxChar th char received
  * @param  size
  * @param  error
  */
void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error);

/**
  * @brief  CNotifies the upper layer that a character has been received
  */
static void (*NotifyCb)(void) = NULL;

/**
  * @brief  Remove backspace and its preceding character in the Command string
  * @param  cmd string to process
  * @retval 0 when OK, otherwise error
  */
static int32_t CMD_ProcessBackSpace(char *cmd);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
void CMD_Init(void (*CmdProcessNotify)(void)) {
    /* USER CODE BEGIN CMD_Init_1 */

    /* USER CODE END CMD_Init_1 */
    if (CmdProcessNotify != NULL) {
        NotifyCb = CmdProcessNotify;
    }
    widx = 0;
    ridx = 0;
    charCount = 0;
    i = 0;
    circBuffOverflow = 0;
    /* USER CODE BEGIN CMD_Init_2 */

    /* USER CODE END CMD_Init_2 */
}

void CMD_Process(void) {
    /* USER CODE BEGIN CMD_Process_1 */

    /* USER CODE END CMD_Process_1 */
    /* Process all commands */
    if (circBuffOverflow == 1) {
        com_error(AT_TEST_PARAM_OVERFLOW);
        /*Full flush in case of overflow */
        __disable_irq();
        ridx = widx;
        charCount = 0;
        circBuffOverflow = 0;
        __enable_irq();
        i = 0;
    }

    while (charCount != 0) {
#if 0  /* echo On    */
    printf("%c", circBuffer[ridx]);
#endif /* 0 */

        // if (circBuffer[ridx] == AT_ERROR_RX_CHAR) {
        //     ridx++;
        //     if (ridx == CIRC_BUFF_SIZE) {
        //         ridx = 0;
        //     }
        //     __disable_irq();
        //     charCount--;
        //     __enable_irq();
        //     com_error(AT_RX_ERROR);
        //     i = 0;
        // } else 
        if ((circBuffer[ridx] == '\r') || (circBuffer[ridx] == '\n')) {
            ridx++;
            if (ridx == CIRC_BUFF_SIZE) {
                ridx = 0;
            }
            __disable_irq();
            charCount--;
            __enable_irq();

            if (i != 0) {
                command[i] = '\0';
                __disable_irq();
                CMD_ProcessBackSpace(command);
                __enable_irq();
                parse_cmd(command);
                i = 0;
            }
        } else if (i == (CMD_SIZE - 1)) {
            i = 0;
            com_error(AT_TEST_PARAM_OVERFLOW);
        } else {
            command[i++] = circBuffer[ridx++];
            if (ridx == CIRC_BUFF_SIZE) {
                ridx = 0;
            }
            __disable_irq();
            charCount--;
            __enable_irq();
        }
    }
    /* USER CODE BEGIN CMD_Process_2 */

    /* USER CODE END CMD_Process_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
static int32_t CMD_ProcessBackSpace(char *cmd) {
    /* USER CODE BEGIN CMD_ProcessBackSpace_1 */

    /* USER CODE END CMD_ProcessBackSpace_1 */
    uint32_t i = 0;
    uint32_t bs_cnt = 0;
    uint32_t cmd_len = 0;
    /*get command length and number of backspace*/
    while (cmd[cmd_len] != '\0') {
        if (cmd[cmd_len] == '\b') {
            bs_cnt++;
        }
        cmd_len++;
    }
    /*for every backspace, remove backspace and its preceding character*/
    for (i = 0; i < bs_cnt; i++) {
        int32_t curs = 0;
        int32_t j = 0;

        /*set cursor to backspace*/
        while (cmd[curs] != '\b') {
            curs++;
        }
        if (curs > 0) {
            for (j = curs - 1; j < cmd_len - 2; j++) {
                cmd[j] = cmd[j + 2];
            }
            cmd[j++] = '\0';
            cmd[j++] = '\0';
            cmd_len -= 2;
        } else {
            return -1;
        }
    }
    return 0;
    /* USER CODE BEGIN CMD_ProcessBackSpace_2 */

    /* USER CODE END CMD_ProcessBackSpace_2 */
}

void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error) {
    /* USER CODE BEGIN CMD_GetChar_1 */

    /* USER CODE END CMD_GetChar_1 */
    charCount++;
    if (charCount == (CIRC_BUFF_SIZE + 1)) {
        circBuffOverflow = 1;
        charCount--;
    } else {
        circBuffer[widx++] = *rxChar;
        if (widx == CIRC_BUFF_SIZE) {
            widx = 0;
        }
    }

    if (NotifyCb != NULL) {
        NotifyCb();
    }
    /* USER CODE BEGIN CMD_GetChar_2 */

    /* USER CODE END CMD_GetChar_2 */
}

static void parse_cmd(const char *cmd) {
    /* USER CODE BEGIN parse_cmd_1 */

    /* USER CODE END parse_cmd_1 */
    ATEerror_t status = AT_OK;
    const struct ATCommand_s *Current_ATCommand;
    int32_t i;

    if ((cmd[0] != 'A') || (cmd[1] != 'T')) {
        status = AT_ERROR;
    } else if (cmd[2] == '\0') {
        /* status = AT_OK; */
    }
    else if (cmd[2] == '?') {
#ifdef NO_HELP
#else
        printf("AT+<CMD>?        : Help on <CMD>\r\n"
                   "AT+<CMD>         : Run <CMD>\r\n"
                   "AT+<CMD>=<value> : Set the value\r\n"
                   "AT+<CMD>=?       : Get the value\r\n");
        for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++) {
            printf(ATCommand[i].help_string);
        }
#endif /* !NO_HELP */
    } else {
        /* point to the start of the command, excluding AT */
        status = AT_ERROR;
        cmd += 2;
        for (i = 0; i < (sizeof(ATCommand) / sizeof(struct ATCommand_s)); i++) {
            if (strncmp(cmd, ATCommand[i].string, ATCommand[i].size_string) == 0) {
                Current_ATCommand = &(ATCommand[i]);
                /* point to the string after the command to parse it */
                cmd += Current_ATCommand->size_string;

                /* parse after the command */
                switch (cmd[0]) {
                case '\0': /* nothing after the command */
                    status = Current_ATCommand->run(cmd);
                    break;
                case '=':
                    if ((cmd[1] == '?') && (cmd[2] == '\0')) {
                        status = Current_ATCommand->get(cmd + 1);
                    } else {
                        status = Current_ATCommand->set(cmd + 1);
                    }
                    break;
                case '?':
#ifndef NO_HELP
                    printf(Current_ATCommand->help_string);
#endif /* !NO_HELP */
                    status = AT_OK;
                    break;
                default:
                    /* not recognized */
                    break;
                }

                /* we end the loop as the command was found */
                break;
            }
        }
    }

    com_error(status);
    /* USER CODE BEGIN parse_cmd_2 */

    /* USER CODE END parse_cmd_2 */
}

static void com_error(ATEerror_t error_type) {
    /* USER CODE BEGIN com_error_1 */

    /* USER CODE END com_error_1 */
    if (error_type > AT_MAX) {
        error_type = AT_MAX;
    }
    printf(ATError_description[error_type]);
    /* USER CODE BEGIN com_error_2 */

    /* USER CODE END com_error_2 */
}