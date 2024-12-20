#pragma once

#include "lora_at.h"
#include <string.h>

#define CMD_SIZE                        1024
#define CIRC_BUFF_SIZE                  32

struct ATCommand_s {
    const char *string;                       /*< command string, after the "AT" */
    const int32_t size_string;                /*< size of the command string, not including the final \0 */
    ATEerror_t (*get)(const char *param);     /*< =? after the string to get the current value*/
    ATEerror_t (*set)(const char *param);     /*< = (but not =?\0) after the string to set a value */
    ATEerror_t (*run)(const char *param);     /*< \0 after the string - run the command */
    #if !defined(NO_HELP)
        const char *help_string;                  /*< to be printed when ? after the string */
    #endif /* !NO_HELP */
};

void CMD_Init(void (*CmdProcessNotify)(void));
void CMD_GetChar(uint8_t *rxChar, uint16_t size, uint8_t error);
void CMD_Process(void);