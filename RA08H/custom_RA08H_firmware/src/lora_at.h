#pragma once

#include "custom_tasks.h"
#include "tremo_system.h"

typedef enum eATEerror {
    AT_OK = 0,
    /* USER CODE BEGIN AT_BLANK_DEF */
    AT_BLANK,
    /* USER CODE END AT_BLANK_DEF */
    AT_ERROR,
    AT_PARAM_ERROR,
    AT_BUSY_ERROR,
    AT_TEST_PARAM_OVERFLOW,
    AT_NO_NET_JOINED,
    AT_RX_ERROR,
    /* USER CODE BEGIN AT_TX_ERROR_DEF */
    AT_TX_ERROR,
    /* USER CODE BEGIN AT_TX_ERROR_DEF */
    AT_NO_CLASS_B_ENABLE,
    AT_DUTYCYCLE_RESTRICTED,
    AT_CRYPTO_ERROR,
    AT_MAX,
} ATEerror_t;

#define AT_RESET      "Z"
#define AT_CCONF      "+CCONF"
#define AT_CTX        "+CTX"
#define AT_CRX        "+CRX"
#define AT_COFF       "+COFF"
#define AT_CLOWPOWER       "+CLOWPOWER"

ATEerror_t AT_reset(const char *param);
ATEerror_t AT_return_ok(const char *param);
ATEerror_t AT_return_error(const char *param);
ATEerror_t AT_custom_get_config(const char *param);
ATEerror_t AT_custom_set_config(const char *param);
ATEerror_t AT_custom_tx(const char *param);
ATEerror_t AT_custom_rx(const char *param);
ATEerror_t AT_custom_off(const char *param);
ATEerror_t AT_custom_low_power(const char *param);