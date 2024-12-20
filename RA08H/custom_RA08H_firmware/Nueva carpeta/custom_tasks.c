#include "custom_tasks.h"

static custom_parameter_t custom_param = {F_915MHz, EMISSION_POWER, custom_BW_250kHz, SF12, CR4o5, 1, DEFAULT_LDR_OPT};

static const uint8_t bandwidth_map[] = {9, 7, 5, 3, 0, 1, 2};

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
static void custom_on_rx_done(const uint8_t *packet, const uint16_t size, const int16_t rssi, const int8_t snr);

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

static void memcpy_8( void *dst, const void *src, uint16_t size )
{
  uint8_t* dst8= (uint8_t *) dst;
  uint8_t* src8= (uint8_t *) src;

  while( size-- )
    {
        *dst8++ = *src8++;
    }
}

int32_t custom_set_config(const custom_parameter_t *param) {
	memcpy_8(&custom_param, param, sizeof(custom_parameter_t));
	return 0;
}

int32_t custom_get_config(custom_parameter_t *param) {
	memcpy_8(param, &custom_param, sizeof(custom_parameter_t));
	return 0;
}

int32_t custom_off() {
	// sleeping will stop receiving
	Radio.Sleep();
	printf("Idle...\r\n");
	return 0;
}

int32_t custom_tx_start(const uint8_t* packet, const uint16_t packet_size) {
	if (packet_size > LORA_MAX_PACKET_SIZE) {
		return -2;
	}

    radio_tx_done_flag = 0;
    radio_tx_timeout_flag = 0;


    radio_events.TxDone = custom_on_tx_done;
    radio_events.RxDone = custom_on_rx_done;
    radio_events.TxTimeout = custom_on_tx_timeout;
    radio_events.RxTimeout = custom_on_rx_timeout;
    radio_events.RxError = custom_on_rx_error;
    Radio.Init(&radio_events);

    Radio.SetChannel(custom_param.freq);

	// lora modulation
	/*Lora*/
    const uint8_t bandwidth_mapped = bandwidth_map[(uint8_t)custom_param.bandwidth];
    const uint8_t spreading_factor = custom_param.loraSf_datarate;
    const uint8_t coding_rate = custom_param.codingRate;
    const int8_t power = custom_param.power;
    uint8_t low_dr_opt = custom_param.lowDrOpt;
    if (low_dr_opt == 2) { // AUTO
        low_dr_opt = ((spreading_factor == 11) || (spreading_factor == 12)) ? 1 : 0;
    }

    Radio.SetTxConfig(MODEM_LORA, power, 0, bandwidth_mapped, spreading_factor, coding_rate, LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON, CRC_ON, 0, 0, LORA_IQ_INVERSION_OFF, TX_TIMEOUT_VALUE);
    
    SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize = low_dr_opt;
    SX126xSetModulationParams(&SX126x.ModulationParams);
    
    Radio.SetPublicNetwork(false); // set private syncword

    Radio.Send(packet, packet_size);

    // wait for TX done/timeout
    while ((!radio_tx_done_flag) && (!radio_tx_timeout_flag)) {}
    Radio.Sleep();

    if (radio_tx_done_flag == 1) {
    	printf("OnTxDone\r\n");
    	return 0;
    }

    if (radio_tx_timeout_flag == 1) {
    	printf("OnTxTimeout\r\n");
    	return -1;
    }

    return 0;
}

int32_t custom_rx_start() {
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
    const uint8_t bandwidth_mapped = bandwidth_map[(uint8_t)custom_param.bandwidth];
    const uint8_t spreading_factor = custom_param.loraSf_datarate;
    const uint8_t coding_rate = custom_param.codingRate;
    uint8_t low_dr_opt = custom_param.lowDrOpt;
    if (low_dr_opt == 2) { // AUTO
        low_dr_opt = ((spreading_factor == 11) || (spreading_factor == 12)) ? 1 : 0;
    }

    Radio.SetRxConfig(MODEM_LORA, bandwidth_mapped, spreading_factor, coding_rate, 0, LORA_PREAMBLE_LENGTH, LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, 0, CRC_ON, 0, 0, LORA_IQ_INVERSION_OFF, true);
    
    SX126x.ModulationParams.Params.LoRa.LowDatarateOptimize = low_dr_opt;
    SX126xSetModulationParams(&SX126x.ModulationParams);

    Radio.SetPublicNetwork(false); // set private syncword

    if (custom_param.lna == 0) {
    	Radio.Rx(RX_TIMEOUT_VALUE);
    } else {
    	Radio.RxBoosted(RX_TIMEOUT_VALUE);
    }

    printf("Receiving...\r\n");

    return 0;
}

void custom_on_tx_done() {
	radio_tx_done_flag = 1;
}

void custom_on_rx_done(const uint8_t *packet, const uint16_t size, const int16_t rssi, const int8_t snr) {
	printf("OnRxDone\r\n");
	printf("Recv:\r\n");

	for (uint16_t i = 0; i < size; i++) {
		printf("%02x ", packet[i]);
	}

	printf("\r\nData end\r\nrssi = %d dBm, snr = %d dB\r\n", rssi, snr);
}

void custom_on_tx_timeout() {
	radio_tx_timeout_flag = 1;
}

void custom_on_rx_timeout() {
	printf("OnRxTimeout\r\n");
}

void custom_on_rx_error() {
	printf("OnRxError\r\n");
}