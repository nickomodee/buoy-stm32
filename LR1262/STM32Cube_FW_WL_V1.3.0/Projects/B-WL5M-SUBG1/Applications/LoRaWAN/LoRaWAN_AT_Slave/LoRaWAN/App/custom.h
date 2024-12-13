/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "lora_at.h" // for `AT_PPRINTF()` macro

#define F_915MHz                      915000000
#define P_14dBm                       14
#define P_22dBm                       22
#define SF12                          12
#define CR4o5                         1
#define EMISSION_POWER                P_22dBm
#define CONTINUOUS_TIMEOUT            0xFFFF
#define LORA_PREAMBLE_LENGTH          8         // same for TX and RX
#define LORA_SYMBOL_TIMEOUT           30        // symbols
#define TX_TIMEOUT_VALUE              20000
#define LORA_FIX_LENGTH_packet_OFF    false
#define LORA_IQ_INVERSION_OFF         false
#define RX_TIMEOUT_VALUE              0 // CONTINUOUS
#define RX_CONTINUOUS_ON              1
#define DEFAULT_LDR_OPT               2

typedef enum
{
  custom_BW_7kHz = 0,
  custom_BW_12kHz = 1,
  custom_BW_31kHz = 2,
  custom_BW_62kHz = 3,
  custom_BW_125kHz = 4,
  custom_BW_250kHz = 5,
  custom_BW_500kHz = 6,
} custom_Lora_BandWidth_t;

typedef struct
{
  uint32_t freq;               // in Hz
  int32_t power;               // [-9 :22]dBm
  uint32_t bandwidth;          // LoRa [0:7.8125, 1: 15.625, 2: 31.25, 3: 62.5, 4: 125, 5: 250, 6: 500]kHz

  uint32_t loraSf_datarate;    // LoRa[SF5..SF12]
  uint32_t codingRate;         // LoRa Only [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
  uint32_t lna;                // 0:off 1:On

  uint32_t lowDrOpt;           // LoRa Only 0: off, 1:On, 2: Auto (1 when SF11 or SF12, 0 otherwise)
} custom_parameter_t;

int32_t custom_set_config(const custom_parameter_t *param);

int32_t custom_get_config(custom_parameter_t *param);

int32_t custom_off();

int32_t custom_tx_start(const uint8_t* packet, const uint16_t packet_size);

int32_t custom_rx_start();

#ifdef __cplusplus
}
#endif
