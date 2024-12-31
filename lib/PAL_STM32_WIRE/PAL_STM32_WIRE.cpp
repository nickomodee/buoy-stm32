#include "PAL_STM32_WIRE.h"

uint8_t PAL_STM32_WIRE::tx_address_ = 0;
bool PAL_STM32_WIRE::transmitting_ = false;
PAL_STM32_STREAM_BUFFER PAL_STM32_WIRE::tx_buffer_(PAL_STM32_WIRE::TX_BUFFER_SIZE);
const I2C_TypeDef* PAL_STM32_WIRE::I2C_instance_ = I2C1;
volatile I2C_HandleTypeDef PAL_STM32_WIRE::hi2c_;

static void I2C_MspInit(const I2C_TypeDef* instance) {
    GPIO_InitTypeDef GPIO_InitStruct = {};
    if (instance == I2C1) {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**I2C1 GPIO Configuration
        PB7     ------> I2C1_SDA
        PB6     ------> I2C1_SCL
        */
        GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* Peripheral clock enable */
        __HAL_RCC_I2C1_CLK_ENABLE();
        // We don't need interrupts, unlike with UART
        // /* I2C1 interrupt Init */
        // HAL_NVIC_SetPriority(I2C1_EV_IRQn, 0, 0);
        // HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
        // HAL_NVIC_SetPriority(I2C1_ER_IRQn, 0, 0);
        // HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    }

}

PAL_STM32_WIRE::PAL_STM32_WIRE() : PAL_STM32_STREAM(RX_BUFFER_SIZE) {};

bool PAL_STM32_WIRE::begin() {
    this->hi2c_.Instance = I2C1;
    this->hi2c_.Init.Timing = 0x00201D2B; // 10 kHz: 0x10108CFF; 50 kHz: 0x00201E7A; 100 kHz: 0x00201D2B; 400 kHz: 0x0010020A;
    this->hi2c_.Init.OwnAddress1 = 0;
    this->hi2c_.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    this->hi2c_.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    this->hi2c_.Init.OwnAddress2 = 0;
    this->hi2c_.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    this->hi2c_.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    this->hi2c_.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    // MSP Init
    I2C_MspInit(this->I2C_instance_);

    // Initialise I2C Peripheral
    if (HAL_I2C_Init(PAL_STM32_WIRE::get_hi2c_ptr()) != HAL_OK) {
        return false;
    }

    /** Configure Analogue filter
     */
    if (HAL_I2CEx_ConfigAnalogFilter(PAL_STM32_WIRE::get_hi2c_ptr(), I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        return false;
    }

    /** Configure Digital filter
     */
    return HAL_I2CEx_ConfigDigitalFilter(PAL_STM32_WIRE::get_hi2c_ptr(), 0) == HAL_OK;
}

void PAL_STM32_WIRE::beginTransmission(const uint8_t address) {
    PAL_STM32_WIRE::tx_address_ = address << 1; // 7-bit address needs to be 8-bit
    PAL_STM32_WIRE::transmitting_ = true;
    PAL_STM32_WIRE::tx_buffer_.reset();
}

uint8_t PAL_STM32_WIRE::endTransmission() {
    uint8_t* data_ptr = const_cast<uint8_t*>(PAL_STM32_WIRE::tx_buffer_.get_read_ptr()); // for some reason it is not marked as const in `HAL_I2C_Master_Transmit()` even though it is not modified
    const uint16_t data_size = PAL_STM32_WIRE::tx_buffer_.get_count();
    const HAL_StatusTypeDef tx_status = HAL_I2C_Master_Transmit(PAL_STM32_WIRE::get_hi2c_ptr(), PAL_STM32_WIRE::tx_address_, data_ptr, data_size, WIRE_MAX_TIMEOUT);
    watchdog.refresh();
    PAL_STM32_WIRE::transmitting_ = false;
    PAL_STM32_WIRE::tx_buffer_.reset();

    switch (tx_status) {
        case HAL_OK:
            return 0; // success
        case HAL_ERROR:
            return 3;   // NACK received or some error in transmission
        case HAL_BUSY:
            return 4;   // line busy
        default:
            // Error_Handler(); // this will never happen, as `HAL_TIMEOUT` can not return from `HAL_I2C_Master_Transmit()`
            return -1;   // make compiler happy, this never happens
    }

    return (uint8_t)tx_status;
}

uint8_t PAL_STM32_WIRE::endTransmission(const bool send_stop) {
    (void)send_stop; // unused (compatible with Arduino libraries), but prevent compiler complaining
    return endTransmission();
}

size_t PAL_STM32_WIRE::requestFrom(const uint8_t address, size_t size) {
    (void)address; // compiler complains because it is unused but why is it unused?? check arduino Wire library...
    if (size > PAL_STM32_WIRE::RX_BUFFER_SIZE) {
        size = PAL_STM32_WIRE::RX_BUFFER_SIZE;
    }
    this->rx_buffer_.reset();
    size_t remaining = this->rx_buffer_.remaining_capacity();
    if (size > remaining) {
        size = remaining;
    }
    uint8_t* write_ptr = const_cast<uint8_t*>(this->rx_buffer_.get_write_ptr()); // I mean this isn't good, but I'd rather use the memory used for the rx buffer in base `Stream` instead of just using an extra array
    const HAL_StatusTypeDef receive_status = HAL_I2C_Master_Receive(PAL_STM32_WIRE::get_hi2c_ptr(), PAL_STM32_WIRE::tx_address_, write_ptr, size, WIRE_MAX_TIMEOUT);
    watchdog.refresh();
    if (receive_status != HAL_OK) {
        return 0;
    }
    this->rx_buffer_.set_count(this->rx_buffer_.get_count() + size);
    return size;
}

size_t PAL_STM32_WIRE::requestFrom(const uint8_t address, size_t size, const bool send_stop) {
    (void)send_stop; // unused (compatible with Arduino libraries), but prevent compiler complaining
    return requestFrom(address, size);
}

size_t PAL_STM32_WIRE::write(const uint8_t data) {
    if (PAL_STM32_WIRE::transmitting_) {
        if (PAL_STM32_WIRE::tx_buffer_.get_count() >= PAL_STM32_WIRE::TX_BUFFER_SIZE) {
            // skip error, maybe implement later?
            return 0;
        }
        PAL_STM32_WIRE::tx_buffer_.put(data);
    } else {
        HAL_I2C_Slave_Transmit(PAL_STM32_WIRE::get_hi2c_ptr(), const_cast<uint8_t*>(&data), 1, WIRE_MAX_TIMEOUT);
        watchdog.refresh();
    }
    return 1;
}

void PAL_STM32_WIRE::flush() {
    this->rx_buffer_.empty();
    PAL_STM32_WIRE::tx_buffer_.empty();
}

I2C_HandleTypeDef* PAL_STM32_WIRE::get_hi2c_ptr() {
    return const_cast<I2C_HandleTypeDef*>(&PAL_STM32_WIRE::hi2c_);
}