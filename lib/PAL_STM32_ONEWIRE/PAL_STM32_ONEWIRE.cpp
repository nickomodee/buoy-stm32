#include "PAL_STM32_ONEWIRE.h"

PAL_STM32_ONEWIRE::PAL_STM32_ONEWIRE() {};

PAL_STM32_ONEWIRE::PAL_STM32_ONEWIRE(const uint8_t physical_pin) {
    begin(physical_pin);
}

static GPIO_TypeDef* get_GPIO_port(const uint8_t physical_pin) {
    return pin_map[physical_pin].GPIO_port;
}

static uint16_t get_GPIO_pin(const uint8_t physical_pin) {
    return pin_map[physical_pin].GPIO_pin;
}

void PAL_STM32_ONEWIRE::begin(const uint8_t physical_pin) {
    GPIO_port_ = get_GPIO_port(physical_pin);
    GPIO_pin_ = get_GPIO_pin(physical_pin);
    
    GPIO_init_ = {0};

    // GPIO Ports Clock Enable
    if (GPIO_port_ == GPIOA) {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOB) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOC) {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOD) {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    } else if (GPIO_port_ == GPIOF) {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    } else {
        return;
    }

    GPIO_init_.Pin = GPIO_pin_;
    GPIO_init_.Pull = GPIO_PULLUP;
    GPIO_init_.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_init_.Mode = GPIO_MODE_INPUT;

    HAL_GPIO_Init(GPIO_port_, &GPIO_init_);
}

void PAL_STM32_ONEWIRE::set_input() {
    const uint8_t pin_number = GET_PIN_NUMBER(GPIO_pin_);
    GPIO_port_->MODER &= ~(0x03 << (pin_number << 1));
}

void PAL_STM32_ONEWIRE::set_output() {
    const uint8_t pin_number = GET_PIN_NUMBER(GPIO_pin_);
    GPIO_port_->MODER = (GPIO_port_->MODER & ~(0x03 << (pin_number << 1))) | (0x01 << (pin_number << 1));
}

bool PAL_STM32_ONEWIRE::read_pin() {
    return (bool)(GPIO_port_->IDR & GPIO_pin_);
}

void PAL_STM32_ONEWIRE::write_pin(const GPIO_PinState state) {
    GPIO_port_->BSRR = ((uint32_t)GPIO_pin_) << (16 * (state == GPIO_PIN_RESET));
}

void PAL_STM32_ONEWIRE::reset_search() {
    // reset the search state
    last_discrepancy = 0;
    last_device_flag = false;
    last_family_discrepancy = 0;
    memset(ROM_NO, 0, sizeof(ROM_NO));
}

bool PAL_STM32_ONEWIRE::reset() {
    set_input();

    // wait until the wire is high... just in case
    for (uint8_t retries = 0; retries < 125; retries++) {
        PAL_STM32_DELAY_US(2);
        if (read_pin()) {
            break;
        }
    }

    set_output();
    write_pin(GPIO_PIN_RESET); // drive output low

    PAL_STM32_DELAY_US(480);

    set_input();
    PAL_STM32_DELAY_US(70);
    const uint8_t presence = !read_pin();
    
    PAL_STM32_DELAY_US(410);
    return presence;
}

void PAL_STM32_ONEWIRE::write_bit(const uint8_t value) {
    if (value & 1) {
		set_output();
		write_pin(GPIO_PIN_RESET); // drive output low
		PAL_STM32_DELAY_US(10);
		set_input(); // drive output high
		PAL_STM32_DELAY_US(55);
	} else {
		set_output();
		write_pin(GPIO_PIN_RESET); // drive output low
		PAL_STM32_DELAY_US(65);
		set_input(); // drive output high
		PAL_STM32_DELAY_US(5);
	}
}

uint8_t PAL_STM32_ONEWIRE::read_bit() {
    set_output();
    write_pin(GPIO_PIN_RESET);
    PAL_STM32_DELAY_US(3);
    set_input(); // let pin float, pull-up will raise
    PAL_STM32_DELAY_US(10);
    const uint8_t value = read_pin();
    PAL_STM32_DELAY_US(53);
    return value;
}

void PAL_STM32_ONEWIRE::write(const uint8_t value, const bool power) {
    for (uint8_t bit_mask = 1; bit_mask; bit_mask <<= 1) {
        write_bit((value & bit_mask) ? 1 : 0); 
    }

    if (!power) {
        write_pin(GPIO_PIN_RESET);
        set_input();
    }
}

void PAL_STM32_ONEWIRE::write_bytes(const uint8_t *buffer, const uint16_t buffer_size, const bool power) {
    for (uint16_t i = 0; i < buffer_size; i++) {
        write(buffer[i]);
    }
    if (!power) {
        write_pin(GPIO_PIN_RESET);
        set_input();
    }
}

uint8_t PAL_STM32_ONEWIRE::read() {
    uint8_t value = 0;
    for (uint8_t bit_mask = 1; bit_mask; bit_mask <<= 1) {
        if (read_bit()) {
            value |= bit_mask;
        }
    }
    return value;
}

void PAL_STM32_ONEWIRE::read_bytes(uint8_t* buffer, uint16_t buffer_size) {
    for (uint16_t i = 0; i < buffer_size; i++) {
        buffer[i] = read();
    }
}

void PAL_STM32_ONEWIRE::select(const uint8_t rom[8]) {
    write(ONEWIRE_CHOOSE_ROM);
    for (uint8_t i = 0; i < 8; i++) {
        write(rom[i]);
    }
}

void PAL_STM32_ONEWIRE::skip() {
    write(ONEWIRE_SKIP_ROM);
}

void PAL_STM32_ONEWIRE::depower() {
    set_input();
}

void PAL_STM32_ONEWIRE::target_search(const uint8_t family_code) {
    ROM_NO[0] = family_code; // set the search state to find SearchFamily type devices
    memset(ROM_NO + 1, 0, 7); // clear the rest of the buffer
    last_discrepancy = 64;
    last_family_discrepancy = 0;
    last_device_flag = false;
}

bool PAL_STM32_ONEWIRE::search(uint8_t* new_address, const bool search_mode) {
    bool search_result = false;

    // if the last call was not the last one
    if (!last_device_flag) {
        // OneWire reset
        if (!reset()) {
            // reset the search
            last_discrepancy = 0;
            last_device_flag = false;
            last_family_discrepancy = 0;
            return false;
        }

        // issue the search command
        if (search_mode) {
            write(ONEWIRE_NORMAL_SEARCH);
        } else {
            write(ONEWIRE_CONDITIONAL_SEARCH);
        }

        // initialise for search
        uint8_t id_bit_number = 1;
        uint8_t last_zero = 0;
        uint8_t rom_byte_number = 0;
        uint8_t rom_byte_mask = 1;
        // loop to do the search
        while (rom_byte_number < 8) {
            uint8_t search_direction;

            // read a bit and its complement
            uint8_t id_bit = read_bit();
            uint8_t cmp_id_bit = read_bit();

            // check for no devices on OneWire
            if ((id_bit == 1) && (cmp_id_bit == 1)) {
                break;
            }

            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit) {
                search_direction = id_bit; // bit write value for search
            } else {
                // if this discrepancy is before the last discrepancy on a previous next then pick the same as last time
                if (id_bit_number < last_discrepancy) {
                    search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                } else {
                    // if equal to last pick 1, if not then pick 0
                    search_direction = (id_bit_number == last_discrepancy);
                }
                // if 0 was picked then record its position in last_zero
                if (search_direction == 0) {
                    last_zero = id_bit_number;

                    // check for last discrepancy in family
                    if(last_zero < 9) {
                        last_family_discrepancy = last_zero;
                    }
                }
            }

            // set or clear the bit in the ROM byte `rom_byte_number` with mask `rom_byte_mask`
            if (search_direction == 1) {
                ROM_NO[rom_byte_number] |= rom_byte_mask;
            } else {
                ROM_NO[rom_byte_number] &= ~rom_byte_mask;
            }

            // serial number search direction write bit
            write_bit(search_direction);

            // increment the byte counter `id_bit_number` and shift the mask `rom_byte_mask`
            id_bit_number++;
            rom_byte_mask <<= 1;
            
            // if the mask is 0 then go to new SerialNum byte `rom_byte_number` and reset mask
            if (rom_byte_mask == 0) {
                rom_byte_number++;
                rom_byte_mask = 1;
            }
        }

        // if the search was successful then
        if (!(id_bit_number < 65)) {
            // search successful so set `last_discrepancy`, `last_device_flag`, and `search_result`
            last_discrepancy = last_zero;

            // check for last device
            if (last_discrepancy == 0) {
                last_device_flag = true;
            }
            search_result = true;
        }
    }

    // if no device found then reset counters so next 'search' will be like a first
    if (!search_result || !ROM_NO[0]) {
        last_discrepancy = 0;
        last_device_flag = false;
        last_family_discrepancy = 0;
    } else {
        for (uint8_t i = 0; i < 8; i++) {
            new_address[i] = ROM_NO[i];
        }
    }

    return search_result;
}

static const uint8_t dscrc2x16_table[] = {
	0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
	0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
	0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
	0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74
};

uint8_t PAL_STM32_ONEWIRE::crc8(const uint8_t* address, const uint8_t length) {
    uint8_t crc = 0;

    for (uint8_t i = 0; i < length; i++) {
        crc = *address++ ^ crc; // just reusing crc as intermediate
        crc = dscrc2x16_table[crc & 0x0F] ^ dscrc2x16_table[16 + ((crc >> 4) & 0x0F)];
    }

    return crc;
}

bool PAL_STM32_ONEWIRE::check_crc16(const uint8_t* input, const uint16_t length, const uint8_t* inverted_crc, uint16_t crc) {
    crc = ~crc16(input, length, crc);
    return (crc & 0x0F) == inverted_crc[0] && (crc >> 8) == inverted_crc[1];
}

uint16_t PAL_STM32_ONEWIRE::crc16(const uint8_t* input, const uint16_t length, uint16_t crc) {
    static const uint8_t odd_parity[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };

    for (uint16_t i = 0; i < length; i++) {
        // even though we're just copying a byte from the input, we'll be doing 16-bit computation with it
        uint16_t cdata = input[i];
        cdata = (cdata ^ crc) & 0xFF;
        crc >>= 8;

        if (odd_parity[cdata & 0x0F] ^ odd_parity[cdata >> 4]) {
            crc ^= 0xC001;
        }

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
}