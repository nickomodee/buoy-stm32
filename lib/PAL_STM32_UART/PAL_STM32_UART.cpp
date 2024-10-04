#include "PAL_STM32_UART.h"

// PAL_STM32_UART_STREAM --------------------------------------------------------------------------

static PAL_STM32_UART_STREAM* USART1_STREAM = nullptr;
static PAL_STM32_UART_STREAM* USART2_STREAM = nullptr;

PAL_STM32_UART_STREAM::PAL_STM32_UART_STREAM(USART_TypeDef* UART_instance) : PAL_STM32_STREAM(RX_BUFFER_SIZE), UART_instance_(UART_instance) {
	if (UART_instance == USART1) {
        if (USART1_STREAM != nullptr) {
            Error_Handler(); // There should only be a single Stream tied to a USART instance
        }
        USART1_STREAM = this;
    } else if (UART_instance == USART2) {
        if (USART2_STREAM != nullptr) {
            Error_Handler(); // There should only be a single Stream tied to a USART instance
        }
        USART2_STREAM = this;
    } else {
        Error_Handler(); // Must be a valid USART instance for the Nucleo F303k8 (USART1 or USART2)
    }
}

static void UART_MSP_INIT(const USART_TypeDef* UART_instance) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (UART_instance == USART1) {
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10    ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    } else if (UART_instance == USART2) {
        /* Peripheral clock enable */
        __HAL_RCC_USART2_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART2 GPIO Configuration
        PA2     ------> USART2_TX
        PA3     ------> USART2_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART2 interrupt Init */
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
}

void PAL_STM32_UART_STREAM::begin(uint32_t baud_rate) {
    this->huart_.Instance = const_cast<USART_TypeDef*>(this->UART_instance_);
    this->huart_.Init.BaudRate = baud_rate;
    this->huart_.Init.WordLength = UART_WORDLENGTH_8B;
    this->huart_.Init.StopBits = UART_STOPBITS_1;
    this->huart_.Init.Parity = UART_PARITY_NONE;
    this->huart_.Init.Mode = UART_MODE_TX_RX;
    this->huart_.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    this->huart_.Init.OverSampling = UART_OVERSAMPLING_16;
    this->huart_.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    this->huart_.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    // MSP Init
    UART_MSP_INIT(this->UART_instance_);

    // Initialise UART peripheral
    if (HAL_UART_Init(get_huart_ptr()) != HAL_OK) {
        Error_Handler();
    }

    // Start UART reception in interrupt mode
    HAL_UART_Receive_IT(get_huart_ptr(), const_cast<uint8_t*>(&this->rx_byte_), 1);
}

size_t PAL_STM32_UART_STREAM::write(const uint8_t data) {
    STM32_NO_INTERRUPTS();
    if (HAL_UART_Transmit(get_huart_ptr(), const_cast<uint8_t*>(&data), 1, UART_MAX_TIMEOUT)) {
        STM32_INTERRUPTS();
    	return 1;
    }

    STM32_INTERRUPTS();
    return 0;
}

size_t PAL_STM32_UART_STREAM::write(const uint8_t* buffer, const size_t size) {
    STM32_NO_INTERRUPTS();
    if (HAL_UART_Transmit(get_huart_ptr(), const_cast<uint8_t*>(buffer), size, UART_MAX_TIMEOUT)) {
    	STM32_INTERRUPTS();
        return size;
    }

    STM32_INTERRUPTS();
    return 0;
}

// Print functions
size_t PAL_STM32_UART_STREAM::print(const char* str) {
    return write(str);
}

size_t PAL_STM32_UART_STREAM::print(const char n) {
    return write(n);
}

size_t PAL_STM32_UART_STREAM::print(const uint8_t n, const uint8_t base) {
    return print((uint32_t)n, base);
}

size_t PAL_STM32_UART_STREAM::print(const int n, const uint8_t base) {
    return print((int32_t)n, base);
}

size_t PAL_STM32_UART_STREAM::print(const unsigned int n, const uint8_t base) {
    return print((uint32_t)n, base);
}

// Modified from: https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Print.cpp
size_t PAL_STM32_UART_STREAM::print(const int32_t n, const uint8_t base) {
    if (base == 0) {
        return write((uint8_t)n);
    } else if (base == 10) {
        if (n < 0) {
            return print('-') + printNumber(-n, 10);
        }
        return printNumber(n, 10);
    } else {
        return printNumber(n, base);
    }
}

size_t PAL_STM32_UART_STREAM::print(const uint32_t n, const uint8_t base) {
    if (base == 0) {
        return write((uint8_t)n);
    }

    return printNumber(n, base);
}

size_t PAL_STM32_UART_STREAM::print(const double n, uint8_t digits) {
    return printFloat(n, digits);
}

// Modified from: https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Print.cpp
size_t PAL_STM32_UART_STREAM::printNumber(uint32_t n, uint8_t base) {
    char buf[8 * sizeof(uint32_t) + 1]; // Assumes 8-bit chars plus zero byte.
    char *str = &buf[sizeof(buf) - 1];

    *str = '\0';

    // prevent crash if called with base == 1
    if (base < 2) base = 10;

    do {
        char c = n % base;
        n /= base;

        *--str = c < 10 ? c + '0' : c + 'A' - 10;
    } while(n);

    return write(str);
}

// Modified from: https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/Print.cpp
size_t PAL_STM32_UART_STREAM::printFloat(double number, uint8_t digits) {
    size_t n = 0;

    if (std::isnan(number)) return print("nan");
    if (std::isinf(number)) return print("inf");
    if (number > 4294967040.0) return print("ovf");  // constant determined empirically
    if (number < -4294967040.0) return print("ovf");  // constant determined empirically

    // Handle negative numbers
    if (number < 0.0) {
        n += print('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    double rounding = 0.5;
    for (uint8_t i=0; i < digits; ++i)
    rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it
    const uint32_t int_part = (uint32_t)number;
    double remainder = number - (double)int_part;
    n += print(int_part);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0) {
        n += print('.'); 
    }

    // Extract digits from the remainder one at a time
    while (digits-- > 0) {
        remainder *= 10.0;
        unsigned int toPrint = (unsigned int)(remainder);
        n += print(toPrint);
        remainder -= toPrint; 
    } 
  
  return n;
}

size_t PAL_STM32_UART_STREAM::println(const char* str) {
    return print(str) + println();
}

size_t PAL_STM32_UART_STREAM::println(const char n) {
    return print(n) + println();
}

size_t PAL_STM32_UART_STREAM::println(const uint8_t n, const uint8_t base) {
    return print(n, base) + println();
}

size_t PAL_STM32_UART_STREAM::println(const int n, const uint8_t base) {
    return print(n, base) + println();
}

size_t PAL_STM32_UART_STREAM::println(const unsigned int n, const uint8_t base) {
    return print(n, base) + println();
}

size_t PAL_STM32_UART_STREAM::println(const int32_t n, const uint8_t base) {
    return print(n, base) + println();
}

size_t PAL_STM32_UART_STREAM::println(const uint32_t n, const uint8_t base) {
    return print(n, base) + println();
}

size_t PAL_STM32_UART_STREAM::println(const double n, uint8_t digits) {
    return print(n, digits) + println();
}

size_t PAL_STM32_UART_STREAM::println() {
    return write("\r\n");
}

UART_HandleTypeDef* PAL_STM32_UART_STREAM::get_huart_ptr() {
    return const_cast<UART_HandleTypeDef*>(&this->huart_);
}

extern "C" { // BEGIN extern "C"
	// USART1 Interrupt Handler
	void USART1_IRQHandler() {
        STM32_NO_INTERRUPTS();
		// Check if a stream instance exists tied to USART1
		if (USART1_STREAM == nullptr) {
            STM32_INTERRUPTS();
			return;
		}

		PAL_STM32_UART_STREAM& UART_Stream = *USART1_STREAM;

		HAL_UART_IRQHandler(UART_Stream.get_huart_ptr());
        STM32_INTERRUPTS();
	}

    // USART2 Interrupt Handler
    void USART2_IRQHandler() {
        STM32_NO_INTERRUPTS();
		// Check if a stream instance exists tied to USART1
		if (USART2_STREAM == nullptr) {
            STM32_INTERRUPTS();
			return;
		}

		PAL_STM32_UART_STREAM& UART_Stream = *USART2_STREAM;

		HAL_UART_IRQHandler(UART_Stream.get_huart_ptr());
        STM32_INTERRUPTS();
	}
    
	// UART Receive Complete Callback
	void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
        const USART_TypeDef* instance = huart->Instance;
		PAL_STM32_UART_STREAM* UART_STREAM;
		// Check if there is a valid Stream tied to this USART instance
		if (instance == USART1 && USART1_STREAM != nullptr) { // We only want to handle the UART instances set up by the `PAL_STM32_UART_STREAM` class
			UART_STREAM = USART1_STREAM;
		} else if (instance == USART2 && USART2_STREAM != nullptr) { // We only want to handle the UART instances set up by the `PAL_STM32_UART_STREAM` class
            UART_STREAM = USART2_STREAM;
        } else {
            return;
        }

		// Attempt to store the received byte in the buffer
		UART_STREAM->put_buffer(UART_STREAM->get_rx_byte()); // Warning, this overflows, but this is intentional.

		// Restart UART reception
		HAL_UART_Receive_IT(UART_STREAM->get_huart_ptr(), const_cast<uint8_t*>(UART_STREAM->get_rx_byte_ptr()), 1);
	}

    // UART Error Callback (just ignore and resume reception)
    void HAL_UART_ErrorCallback(UART_HandleTypeDef* huart) {
        const USART_TypeDef* instance = huart->Instance;
		PAL_STM32_UART_STREAM* UART_STREAM;
		// Check if there is a valid Stream tied to this USART instance
		if (instance == USART1 && USART1_STREAM != nullptr) { // We only want to handle the UART instances set up by the `PAL_STM32_UART_STREAM` class
			UART_STREAM = USART1_STREAM;
		} else if (instance == USART2 && USART2_STREAM != nullptr) { // We only want to handle the UART instances set up by the `PAL_STM32_UART_STREAM` class
            UART_STREAM = USART2_STREAM;
        } else {
            return;
        }
        
		// Restart UART reception
		HAL_UART_Receive_IT(UART_STREAM->get_huart_ptr(), const_cast<uint8_t*>(UART_STREAM->get_rx_byte_ptr()), 1);
    }
} // END extern "C"