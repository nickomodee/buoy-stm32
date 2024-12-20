#include "lora_at.h"

ATEerror_t AT_reset(const char *param) {
    system_reset();
    return AT_OK; // won't reach here but keeps compiler happy
}

ATEerror_t AT_return_ok(const char *param) {
    return AT_OK;
}

ATEerror_t AT_return_error(const char *param) {
    return AT_ERROR;
}

ATEerror_t AT_custom_get_config(const char *param) {
	custom_parameter_t custom_param;
	uint32_t lora_BW[7] = {7812, 15625, 31250, 62500, 125000, 250000, 500000};

	custom_get_config(&custom_param);

	printf("1: Freq= %lu Hz\r\n", custom_param.freq);
	printf("2: Power= %ld dBm\r\n", custom_param.power);
	printf("3: Bandwidth= %lu (=%lu Hz)\r\n", custom_param.bandwidth, lora_BW[custom_param.bandwidth]);
	printf("4: SF= %lu \r\n", custom_param.loraSf_datarate);
	printf("5: CR= %lu (=4/%lu) \r\n", custom_param.codingRate, custom_param.codingRate + 4);
	printf("6: LNA State= %lu  \r\n", custom_param.lna);
	printf("7: LowDRopt[0 to 2]= %lu \r\n", custom_param.lowDrOpt);

	return AT_OK;
}

ATEerror_t AT_custom_set_config(const char *param) {
	custom_parameter_t custom_param = {0};
	uint32_t freq;
	int32_t power;
	uint32_t bandwidth;
	uint32_t loraSf_datarate;
	uint32_t codingRate;
	uint32_t lna;
	uint32_t lowDrOpt;
	uint32_t crNum;

	if (8 == sscanf(param, "%lu:%ld:%lu:%lu:%lu/%lu:%lu:%lu",
						&freq,
						&power,
						&bandwidth,
						&loraSf_datarate,
						&crNum,
						&codingRate,
						&lna,
						&lowDrOpt)) {
	/*extend to new format for extended ???*/
	} else {
		return AT_PARAM_ERROR;
	}

	if (freq < 1000) { // in MHz
		custom_param.freq = freq * 1000000;
	} else {
		custom_param.freq = freq;
	}

	// power check and set
	if ((power < -9) || (power > 22)) {
		return AT_PARAM_ERROR;
	}
	custom_param.power = power;

	if (bandwidth > custom_BW_500kHz) {
		return AT_PARAM_ERROR;
	} else {
		custom_param.bandwidth = bandwidth;
	}

	// 4: datarate/spreading factor check and set
	if ((loraSf_datarate < 5) || (loraSf_datarate > 12)) {
		return AT_PARAM_ERROR;
	}
	custom_param.loraSf_datarate = loraSf_datarate;

	// 5: coding rate check and set
	if ((codingRate < 5) || (codingRate > 8)) {
		return AT_PARAM_ERROR;
	}
	custom_param.codingRate = codingRate - 4;

	// 6: lna state check and set
	if (lna > 1)
	{
		return AT_PARAM_ERROR;
	}
	custom_param.lna = lna;

	// 7: low datarate optimisation check and set
	if (lowDrOpt > 2) {
		return AT_PARAM_ERROR;
	}
	custom_param.lowDrOpt = lowDrOpt;

	custom_set_config(&custom_param);

	return AT_OK;
}

uint8_t hex_char_to_byte(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    // invalid
    return (uint8_t)-1;
}

#define ARR_SIZE(x) sizeof(x) / sizeof(x[0])

ATEerror_t AT_custom_tx(const char *param) {
	if (param[0] == '\0') {
		return AT_ERROR;
	}

	static uint8_t packet[LORA_MAX_PACKET_SIZE];
	uint16_t packet_index = 0;

	uint16_t i = 0;
	while (i < ARR_SIZE(packet) * 2) { // * 2 because hex uses 2 characters per byte
		if (param[i + 1] == '\0') {
			return AT_ERROR; // uneven hex bytes
		}
		const uint8_t msb = hex_char_to_byte(param[i++]);
		if (msb == (uint8_t)-1) {
			return AT_ERROR; // not a valid hex byte
		}

		const uint8_t lsb = hex_char_to_byte(param[i++]);
		if (lsb == (uint8_t)-1) {
			return AT_ERROR; // not a valid hex byte
		}
		packet[packet_index++] = (msb << 4) | lsb;
		if (param[i] == '\0') { // we have reached the end with valid data
			return (custom_tx_start(packet, packet_index) == 0) ? AT_BLANK : AT_TX_ERROR;
		}
	}
	return AT_PARAM_ERROR; // too many bytes
}

ATEerror_t AT_custom_rx(const char *param) {
	(void)param; // make compiler happy

	if (0 == custom_rx_start()) {
		return AT_BLANK;
	} else {
		return AT_RX_ERROR;
	}
}

ATEerror_t AT_custom_off(const char *param) {
	(void)param; // make compiler happy

	custom_off();
	return AT_BLANK;
}

ATEerror_t AT_custom_low_power(const char *param) {
	(void)param; // make compiler happy

	custom_sleep();
	return AT_BLANK;
}