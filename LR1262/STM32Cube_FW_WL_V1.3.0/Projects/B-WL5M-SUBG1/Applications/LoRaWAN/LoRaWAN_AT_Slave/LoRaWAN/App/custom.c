#include "platform.h"
#include "sys_app.h"
#include "custom.h"
#include "radio.h"
#include "stm32_seq.h"
#include "utilities_def.h"

static custom_parameter_t custom_param = {F_915MHz, EMISSION_POWER, custom_BW_125kHz, SF12, CR4o5, 1, DEFAULT_LDR_OPT};

static __IO uint32_t radio_tx_done_flag = 0;
static __IO uint32_t radio_tx_timeout_flag = 0;

/*!
 * Radio events function pointer
 */
static RadioEvents_t radio_events;

/*!
 * \brief Function to be executed on Radio TX Done event
 */
static void custom_on_tx_done();

/*!
 * \brief Function to be executed on Radio RX Done event
 */
static void custom_on_rx_done(uint8_t *packet, uint16_t size, int16_t rssi, int8_t snr);

/*!
 * \brief Function executed on Radio RX Timeout event
 */
static void custom_on_rx_timeout();

/*!
 * \brief Function executed on Radio TX Timeout event
 */
static void custom_on_tx_timeout();

/*!
 * \brief Function executed on Radio Rx Error event
 */
static void custom_on_rx_error();

int32_t custom_set_config(const custom_parameter_t *param) {
	UTIL_MEM_cpy_8(&custom_param, param, sizeof(custom_parameter_t));
	return 0;
}

int32_t custom_get_config(custom_parameter_t *param) {
	UTIL_MEM_cpy_8(param, &custom_param, sizeof(custom_parameter_t));
	return 0;
}

int32_t custom_off() {
	// sleeping will stop receiving
	Radio.Sleep();
	AT_PPRINTF("Sleeping...\r\n");
	return 0;
}

int32_t custom_tx_start(const uint8_t* packet, const uint16_t packet_size) {
    radio_tx_done_flag = 0;
    radio_tx_timeout_flag = 0;

    TxConfigGeneric_t tx_config;

    radio_events.TxDone = custom_on_tx_done;
    radio_events.RxDone = custom_on_rx_done;
    radio_events.TxTimeout = custom_on_tx_timeout;
    radio_events.RxTimeout = custom_on_rx_timeout;
    radio_events.RxError = custom_on_rx_error;
    Radio.Init(&radio_events);

    Radio.SetChannel(custom_param.freq);

	// lora modulation
	tx_config.lora.Bandwidth = (RADIO_LoRaBandwidths_t)custom_param.bandwidth;
	tx_config.lora.SpreadingFactor = (RADIO_LoRaSpreadingFactors_t)custom_param.loraSf_datarate; /*BitRate*/
	tx_config.lora.Coderate = (RADIO_LoRaCodingRates_t)custom_param.codingRate;
	tx_config.lora.LowDatarateOptimize = (RADIO_Ld_Opt_t)custom_param.lowDrOpt; /*0 inactive, 1 active, 2: auto*/
	tx_config.lora.PreambleLen = LORA_PREAMBLE_LENGTH;
	tx_config.lora.LengthMode = RADIO_LORA_PACKET_VARIABLE_LENGTH;
	tx_config.lora.CrcMode = RADIO_LORA_CRC_ON;
	tx_config.lora.IqInverted = RADIO_LORA_IQ_NORMAL;
	Radio.RadioSetTxGenericConfig(GENERIC_LORA, &tx_config, custom_param.power, TX_TIMEOUT_VALUE);
    Radio.SetPublicNetwork(false); // set private syncword

    Radio.Send(packet, packet_size);

    // wait for TX done/timeout
    UTIL_SEQ_WaitEvt(1 << CFG_SEQ_Evt_RadioOnTstRF);
    Radio.Sleep();

    if (radio_tx_done_flag == 1) {
    	AT_PPRINTF("OnTxDone\r\n");
    	return 0;
    }

    if (radio_tx_timeout_flag == 1) {
    	AT_PPRINTF("OnTxTimeout\r\n");
    	return -1;
    }

    return 0;
}

int32_t custom_rx_start() {
	RxConfigGeneric_t rx_config = {0};

	/* Radio initialization */
    radio_events.TxDone = custom_on_tx_done;
    radio_events.RxDone = custom_on_rx_done;
    radio_events.TxTimeout = custom_on_tx_timeout;
    radio_events.RxTimeout = custom_on_rx_timeout;
    radio_events.RxError = custom_on_rx_error;
    Radio.Init(&radio_events);

    /* Rx config */
    Radio.SetChannel(custom_param.freq);

    /*Lora*/
    rx_config.lora.Bandwidth = (RADIO_LoRaBandwidths_t)custom_param.bandwidth;
    rx_config.lora.SpreadingFactor = (RADIO_LoRaSpreadingFactors_t)custom_param.loraSf_datarate; /*BitRate*/
    rx_config.lora.Coderate = (RADIO_LoRaCodingRates_t)custom_param.codingRate;
    rx_config.lora.LowDatarateOptimize = (RADIO_Ld_Opt_t)custom_param.lowDrOpt; /*0 inactive, 1 active, 2: auto*/
    rx_config.lora.PreambleLen = LORA_PREAMBLE_LENGTH;
    rx_config.lora.LengthMode = RADIO_LORA_PACKET_VARIABLE_LENGTH;
    rx_config.lora.CrcMode = RADIO_LORA_CRC_ON;
    rx_config.lora.IqInverted = RADIO_LORA_IQ_NORMAL;
    Radio.RadioSetRxGenericConfig(GENERIC_LORA, &rx_config, RX_CONTINUOUS_ON, LORA_SYMBOL_TIMEOUT);
    Radio.SetPublicNetwork(false); // set private syncword

    if (custom_param.lna == 0) {
    	Radio.Rx(RX_TIMEOUT_VALUE);
    } else {
    	Radio.RxBoosted(RX_TIMEOUT_VALUE);
    }

    AT_PPRINTF("Receiving...\r\n");

    return 0;
}

void custom_on_tx_done() {
	radio_tx_done_flag = 1;
	UTIL_SEQ_SetEvt(1 << CFG_SEQ_Evt_RadioOnTstRF);
}

void custom_on_rx_done(uint8_t *packet, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo) {
	AT_PPRINTF("OnRxDone\r\n");
	AT_PPRINTF("Recv:\r\n");

	for (uint16_t i = 0; i < size; i++) {
		AT_PPRINTF("%02x ", packet[i]);
	}

	AT_PPRINTF("\r\nData end\r\n\r\n");
}

void custom_on_tx_timeout() {
	radio_tx_timeout_flag = 1;
	UTIL_SEQ_SetEvt(1 << CFG_SEQ_Evt_RadioOnTstRF);
}

void custom_on_rx_timeout() {
	AT_PPRINTF("OnRxTimeout\r\n");
}

void custom_on_rx_error() {
	AT_PPRINTF("OnRxError\r\n");
}
