#include "DS18B20_Base.h"

DS18B20_Base::DS18B20_Base(uint8_t pin) : oneWire(PAL_ONEWIRE(pin)), numberOfDevices(0), globalResolution(0), selectedResolution(0), selectedPowerMode(0) {
    resetSearch();
    sendCommand(SKIP_ROM, READ_POWER_SUPPLY);
    globalPowerMode = oneWire.read_bit();

    while (selectNext()) {
        uint8_t resolution = getResolution();

        if (resolution > globalResolution) {
            globalResolution = resolution;
        }

        numberOfDevices++;
    }
}

uint8_t DS18B20_Base::select(uint8_t address[]) {
    if (isConnected(address)) {
        memcpy(selectedAddress, address, 8);

        if (readScratchpad()) {
            selectedResolution = getResolution();

            sendCommand(MATCH_ROM, READ_POWER_SUPPLY);
            selectedPowerMode = oneWire.read_bit();

            return 1;
        }
    }

    return 0;
}

uint8_t DS18B20_Base::selectNext() {
    if (oneWireSearch(SEARCH_ROM)) {
        return select(searchAddress);
    }

    return 0;
}

uint8_t DS18B20_Base::selectNextAlarm() {
    if (oneWireSearch(ALARM_SEARCH)) {
        return select(searchAddress);
    }

    return 0;
}

void DS18B20_Base::resetSearch() {
    lastDiscrepancy = 0;
    lastDevice = 0;
}

float DS18B20_Base::getTempC() {
    sendCommand(MATCH_ROM, CONVERT_T, !selectedPowerMode);
    delayForConversion(selectedResolution, selectedPowerMode);
    readScratchpad();
    uint8_t lsb = selectedScratchpad[TEMP_LSB];
    uint8_t msb = selectedScratchpad[TEMP_MSB];

    switch (selectedResolution) {
        case 9:
            lsb &= 0xF8;
            break;
        case 10:
            lsb &= 0xFC;
            break;
        case 11:
            lsb &= 0xFE;
            break;
    }

    uint8_t sign = msb & 0x80;
    int16_t temp = (msb << 8) + lsb;

    if (sign) {
        temp = ((temp ^ 0xffff) + 1) * -1;
    }

    return temp / 16.0;
}

float DS18B20_Base::getTempF() {
    return getTempC() * 1.8 + 32;
}

uint8_t DS18B20_Base::getResolution() {
    switch (selectedScratchpad[CONFIGURATION]) {
        case RES_9_BIT:
            return 9;
        case RES_10_BIT:
            return 10;
        case RES_11_BIT:
            return 11;
        case RES_12_BIT:
            return 12;
    }
    return 0;
}

void DS18B20_Base::setResolution(uint8_t resolution) {
    resolution = PAL_CONSTRAIN(resolution, 9, 12);

    switch (resolution) {
        case 9:
            selectedScratchpad[CONFIGURATION] = RES_9_BIT;
            break;
        case 10:
            selectedScratchpad[CONFIGURATION] = RES_10_BIT;
            break;
        case 11:
            selectedScratchpad[CONFIGURATION] = RES_11_BIT;
            break;
        case 12:
            selectedScratchpad[CONFIGURATION] = RES_12_BIT;
            break;
    }

    if (resolution > globalResolution) {
        globalResolution = resolution;
    }

    writeScratchpad();
}

uint8_t DS18B20_Base::getPowerMode() {
    return selectedPowerMode;
}

uint8_t DS18B20_Base::getFamilyCode() {
    return selectedAddress[0];
}

void DS18B20_Base::getAddress(uint8_t address[]) {
    memcpy(address, selectedAddress, 8);
}

void DS18B20_Base::doConversion() {
    sendCommand(SKIP_ROM, CONVERT_T, !globalPowerMode);
    delayForConversion(globalResolution, globalPowerMode);
}

uint8_t DS18B20_Base::getNumberOfDevices() {
    return numberOfDevices;
}

uint8_t DS18B20_Base::hasAlarm() {
    uint8_t oldResolution = selectedResolution;
    setResolution(9);
    float temp = getTempC();
    setResolution(oldResolution);
    return ((temp <= selectedScratchpad[ALARM_LOW]) || (temp >= selectedScratchpad[ALARM_HIGH]));
}

void DS18B20_Base::setAlarms(int8_t alarmLow, int8_t alarmHigh) {
    setAlarmLow(alarmLow);
    setAlarmHigh(alarmHigh);
    writeScratchpad();
}

int8_t DS18B20_Base::getAlarmLow() {
    return selectedScratchpad[ALARM_LOW];
}

void DS18B20_Base::setAlarmLow(int8_t alarmLow) {
    alarmLow = PAL_CONSTRAIN(alarmLow, -55, 125);
    selectedScratchpad[ALARM_LOW] = alarmLow;
    writeScratchpad();
}

int8_t DS18B20_Base::getAlarmHigh() {
    return selectedScratchpad[ALARM_HIGH];
}

void DS18B20_Base::setAlarmHigh(int8_t alarmHigh) {
    alarmHigh = PAL_CONSTRAIN(alarmHigh, -55, 125);
    selectedScratchpad[ALARM_HIGH] = alarmHigh;
    writeScratchpad();
}

void DS18B20_Base::setRegisters(int8_t lowRegister, int8_t highRegister) {
    setAlarms(lowRegister, highRegister);
}

int8_t DS18B20_Base::getLowRegister() {
    return getAlarmLow();
}

void DS18B20_Base::setLowRegister(int8_t lowRegister) {
    setAlarmLow(lowRegister);
}

int8_t DS18B20_Base::getHighRegister() {
    return getAlarmHigh();
}

void DS18B20_Base::setHighRegister(int8_t highRegister) {
    setAlarmHigh(highRegister);
}

uint8_t DS18B20_Base::readScratchpad() {
    sendCommand(MATCH_ROM, READ_SCRATCHPAD);

    for (uint8_t i = 0; i < SIZE_SCRATCHPAD; i++) {
        selectedScratchpad[i] = oneWire.read();
    }

    return PAL_ONEWIRE::crc8(selectedScratchpad, 8) == selectedScratchpad[CRC8];
}

void DS18B20_Base::writeScratchpad() {
    sendCommand(MATCH_ROM, WRITE_SCRATCHPAD);
    oneWire.write(selectedScratchpad[ALARM_HIGH]);
    oneWire.write(selectedScratchpad[ALARM_LOW]);
    oneWire.write(selectedScratchpad[CONFIGURATION]);
    sendCommand(MATCH_ROM, COPY_SCRATCHPAD, !selectedPowerMode);

    if (!selectedPowerMode) {
        PAL_DELAY(10);
    }
}

uint8_t DS18B20_Base::sendCommand(uint8_t romCommand) {
    if (!oneWire.reset()) {
        return 0;
    }

    switch (romCommand) {
        case SEARCH_ROM:
        case SKIP_ROM:
        case ALARM_SEARCH:
            oneWire.write(romCommand);
            break;
        case MATCH_ROM:
            oneWire.select(selectedAddress);
            break;
        default:
            return 0;
    }

    return 1;
}

uint8_t DS18B20_Base::sendCommand(uint8_t romCommand, uint8_t functionCommand, uint8_t power) {
    if (!sendCommand(romCommand)) {
        return 0;
    }

    switch (functionCommand) {
        case CONVERT_T:
        case COPY_SCRATCHPAD:
            oneWire.write(functionCommand, power);
            break;
        case WRITE_SCRATCHPAD:
        case READ_SCRATCHPAD:
        case READ_POWER_SUPPLY:
            oneWire.write(functionCommand);
            break;
        default:
            return 0;
    }

    return 1;
}

uint8_t DS18B20_Base::oneWireSearch(uint8_t romCommand) {
    if (lastDevice || !sendCommand(romCommand)) {
        resetSearch();
        return 0;
    }

    uint8_t lastZero = 0;
    uint8_t direction, byteNumber, bitNumber, currentBit, currentBitComp;

    for (uint8_t bitPosition = 0; bitPosition < 64; bitPosition++) {
        currentBit = oneWire.read_bit();
        currentBitComp = oneWire.read_bit();

        if (currentBit && currentBitComp) {
            lastDiscrepancy = 0;
            return 0;
        }

        byteNumber = bitPosition / 8;
        bitNumber = bitPosition % 8;

        if (!currentBit && !currentBitComp) {
            if (bitPosition == lastDiscrepancy) {
                direction = 1;
            } else if (bitPosition > lastDiscrepancy) {
                direction = 0;
                lastZero = bitPosition;
            } else {
                direction = PAL_BIT_READ(searchAddress[byteNumber], bitNumber);

                if (!direction) {
                    lastZero = bitPosition;
                }
            }
        } else {
            direction = currentBit;
        }

        PAL_BIT_WRITE(searchAddress[byteNumber], bitNumber, direction);
        oneWire.write_bit(direction);
    }

    lastDiscrepancy = lastZero;

    if (!lastDiscrepancy) {
        lastDevice = 1;
    }

    return 1;
}

uint8_t DS18B20_Base::isConnected(uint8_t address[]) {
    if (!sendCommand(SEARCH_ROM)) {
        return 0;
    }

    uint8_t currentBit, currentBitComp, byteNumber, bitNumber;

    for (uint8_t bitPosition = 0; bitPosition < 64; bitPosition++) {
        currentBit = oneWire.read_bit();
        currentBitComp = oneWire.read_bit();

        if (currentBit && currentBitComp) {
            return 0;
        }

        byteNumber = bitPosition / 8;
        bitNumber = bitPosition % 8;
        oneWire.write_bit(PAL_BIT_READ(address[byteNumber], bitNumber));
    }

    return 1;
}

void DS18B20_Base::delayForConversion(uint8_t resolution, uint8_t powerMode) {
    if (powerMode) {
        while (!oneWire.read_bit());
    } else {
        switch (resolution) {
            case 9:
                PAL_DELAY(CONV_TIME_9_BIT);
                break;
            case 10:
                PAL_DELAY(CONV_TIME_10_BIT);
                break;
            case 11:
                PAL_DELAY(CONV_TIME_11_BIT);
                break;
            case 12:
                PAL_DELAY(CONV_TIME_12_BIT);
                break;
        }
    }
}