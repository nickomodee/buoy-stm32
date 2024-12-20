#pragma once

#include "lora_command.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "tremo_lpuart.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_pwr.h"
#include "delay.h"
#include "timer.h"
#include "sx126x-board.h"
#include "sx126x.h"
#include "radio.h"

void custom_main();