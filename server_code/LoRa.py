import serial
import time
from typing import Literal, List
from enum import Enum
import re

import logging
logger = logging.getLogger('[LoRa]')

LORA_MAX_SIZE = 255 # max packet size

class DataRate(Enum):
    SF7 = 7
    SF8 = 8
    SF9 = 9
    SF10 = 10
    SF11 = 11
    SF12 = 12

class Bandwidth(Enum):
    BANDWIDTH_7_81_KHZ = 0
    BANDWIDTH_15_63_KHZ = 1
    BANDWIDTH_31_25_KHZ = 2
    BANDWIDTH_62_5_KHZ = 3
    BANDWIDTH_125_KHZ = 4
    BANDWIDTH_250_KHZ = 5
    BANDWIDTH_500_KHZ = 6

class CodeRate(Enum):
    RATE_4_BY_5 = 5
    RATE_4_BY_6 = 6
    RATE_4_BY_7 = 7
    RATE_4_BY_8 = 8

class LNA(Enum):
    OFF = 0
    ON = 1

class LowDrOpt(Enum):
    OFF = 0
    ON = 1
    AUTO = 2

class LoRaState(Enum):
    RX = 0
    TX = 1
    IDLE = 2
    SLEEP = 3 # same as idle but backwards compatible with RA-08H

AT_CCONF: bytes = b"AT+CCONF"
AT_CTX: bytes = b"AT+CTX"
AT_CRX: bytes = b"AT+CRX"
AT_COFF: bytes = b"AT+COFF"
AT_RESET: bytes = b"ATZ"

def hex_char_to_byte(c: int) -> int:
    if c >= ord('0') and c <= ord('9'):
        return c - ord('0')
    elif c >= ord('a') and c <= ord('f'):
        return c - ord('a') + 10
    elif c >= ord('A') and c <= ord('F'):
        return c - ord('A') + 10
    # invalid
    return -1

class LoRa:
    def __init__(self, lora_serial: serial.Serial, timeout: float, num_retries: int, freq: int, data_rate: DataRate, bandwidth: Bandwidth, code_rate: CodeRate, tx_power: int, lna: LNA, low_dr_opt: LowDrOpt) -> None:
        self._lora_serial: serial.Serial = lora_serial
        self._serial_buffer: bytes = b""
        self._timeout: float = timeout
        self._num_retries: int = num_retries
        self._freq: int = freq
        self._data_rate: DataRate = data_rate
        self._bandwidth: Bandwidth = bandwidth
        self._code_rate: CodeRate = code_rate
        self._tx_power: int = tx_power
        self._lna: LNA = lna
        self._low_dr_opt: LowDrOpt = low_dr_opt
        self._state: LoRaState = LoRaState.IDLE
        self._buffer: bytes = b""
        self._serial_buffer: bytes = b""

    def begin(self) -> bool:
        self._flush_serial()
        if not self.reset():
            return False
        if not self.configure():
            return False
        self._state = LoRaState.IDLE
        return True
    
    def reset(self) -> bool:
        command_buffer: bytes = AT_RESET + b"\r\n"
        expected_buffer: bytes = b"AT? to list all available functions\r\n"
        return self._send_command(command_buffer, expected_buffer)

    def configure(self) -> bool:
        command_buffer = AT_CCONF + f"={self._freq}:{self._tx_power}:{self._bandwidth.value}:{self._data_rate.value}:4/{self._code_rate.value}:{self._lna.value}:{self._low_dr_opt.value}\r\n".encode()
        expected_buffer = b"\r\nOK\r\n"
        return self._send_command(command_buffer, expected_buffer)

    def send(self, data: bytes | List[int]) -> bool:
        if len(data) > LORA_MAX_SIZE:
            return False
        
        SAVED_STATE: LoRaState = self._state
        if not self.set_state(LoRaState.TX):
            self.set_state(SAVED_STATE)
            return False
        
        self._buffer = AT_CTX + b"="
        # append data in 2-byte padded hex format with no separation
        for datum in data:
            self._buffer += f"{datum:02x}".encode()
        
        self._buffer += b"\r\n"

        EXPECTED_BUFER: bytes = b"OnTxDone\r\n"
        if not self._send_command(self._buffer, EXPECTED_BUFER):
            self.set_state(SAVED_STATE)
            return False
        
        self.set_state(SAVED_STATE) # ignore return value, we still successfully sent
        return True

    def set_state(self, updated_state: LoRaState) -> bool:
        if updated_state == LoRaState.RX:
            return self.enter_rx_mode_()
        elif updated_state == LoRaState.TX:
            return self.enter_tx_mode_()
        elif updated_state == LoRaState.IDLE:
            return self.enter_idle_mode_()
        elif updated_state == LoRaState.SLEEP:
            return self.enter_idle_mode_() # `IDLE` is already low power
        else:
            return False

    def get_state(self) -> LoRaState:
        return self._state
    
    def enter_rx_mode_(self) -> bool:
        if not self.configure():
            return False
        command_buffer: bytes = AT_CRX + b"\r\n"
        expected_buffer: bytes = b"Receiving...\r\n"
        return self._send_command(command_buffer, expected_buffer)
    
    def enter_tx_mode_(self) -> bool:
        if not self.enter_idle_mode_():
            return False
        return self.configure()

    def enter_idle_mode_(self) -> bool:
        if not self.configure():
            return False
        command_buffer: bytes = AT_COFF + b"\r\n"
        expected_buffer: bytes = b"Idle...\r\n"
        return self._send_command(command_buffer, expected_buffer)

    def recv(self) -> bool:
        SAVED_STATE: LoRaState = self._state
        def success() -> bool:
            self.set_state(SAVED_STATE)
            return True
        def fail() -> bool:
            self.set_state(SAVED_STATE)
            return False
        if not self.set_state(LoRaState.RX):
            return fail()
        for i in range(self._num_retries):
            self._buffer = b""
            expected_buffer: bytes = b"Recv:\r\n"
            if not self._expected(expected_buffer):
                continue
            c1: int = self._read_blocking()
            if c1 == -1:
                return fail()
            c2: int = self._read_blocking()
            if c2 == -1:
                return fail()
            while c1 != ord('\r'):
                if len(self._buffer) >= LORA_MAX_SIZE:
                    return fail()
                c1_byte = hex_char_to_byte(c1)
                c2_byte = hex_char_to_byte(c2)
                if (c1_byte == -1) or (c2_byte == -1): # invalid hex formats
                    return fail()
                self._buffer += ((c1_byte << 4) | c2_byte).to_bytes(1, byteorder='little')
                if self._read_blocking() != ord(' '): # flush ' ' char
                    return fail()
                c1 = self._read_blocking()
                if c1 == -1:
                    return fail()
                c2 = self._read_blocking()
                if c2 == -1:
                    return fail()
            # the rx ends with `"Data end\r\nrssi = {rssi} dBm, snr = {snr} dB\r\n"`
            expected_buffer = b"Data end\r\nrssi = "
            if not self._expected(expected_buffer):
                return fail()
            expected_buffer = b" dB\r\n"
            if not self._expected(expected_buffer):
                return fail()
            
            regex_match = re.match(rb"([+-]?\d+) dBm, snr = ([+-]?\d+) dB", self._serial_buffer)
            if regex_match:
                rssi = int(regex_match.group(1))
                snr = int(regex_match.group(2))
                logger.debug(f"Received rssi: {rssi}, snr: {snr}")
            else:
                logger.debug(f"Received regex failed: {self._serial_buffer}")

            return success()
        return fail()

    def get_buffer(self) -> bytes:
        return self._buffer

    def _send_command(self, command: bytes, expected: bytes) -> bool:
        self._flush_serial()
        for i in range(self._num_retries):
            self._lora_serial.write(command)
            if not self._expected(expected):
                continue
            return True
        return False

    def _expected(self, expected: bytes) -> bool:
        START_TIME: float = time.time()
        self._serial_buffer = b""
        while len(self._serial_buffer) < len(expected) or self._serial_buffer[-len(expected):] != expected:
            # check if timeout reached
            if (self._lora_serial.in_waiting == 0) and (time.time() - START_TIME >= self._timeout): # `self._lora_serial.in_waiting == 0` so that we don't "cut-off" in the middle of receiving data
                return False
            
            # check if byte available to read (so that we don't block)
            if self._lora_serial.in_waiting:
                self._serial_buffer += self._lora_serial.read(1)
        return True
    
    def _read_blocking(self) -> int:
        START_TIME: float = time.time()
        while not self._lora_serial.in_waiting:
            if time.time() - START_TIME >= self._timeout:
                return -1
        return int(self._lora_serial.read(1)[0])
    
    def _flush_serial(self) -> None:
        if not self._lora_serial.in_waiting: # prevent blocking
            return
        self._lora_serial.read_all()
    
if __name__ == "__main__":
    LORA_COM_PORT: Literal['COM7'] = 'COM9'
    LORA_BAUDRATE: Literal[9600] = 9600
    # LoRa config
    LORA_TIMEOUT: float = 10.0
    LORA_NUM_RETRIES: Literal[5] = 5
    LORA_FREQ: Literal[915000000] = 915000000  # 915 MHz
    LORA_DATA_RATE: DataRate = DataRate.SF12
    LORA_BANDWIDTH: Bandwidth = Bandwidth.BANDWIDTH_125_KHZ
    LORA_CODE_RATE: CodeRate = CodeRate.RATE_4_BY_5
    LORA_TX_POWER: Literal[22] = 22
    LORA_LNA: LNA = LNA.ON
    LORA_LOW_DR_OPT: LowDrOpt = LowDrOpt.AUTO

    lora_serial: serial.Serial = serial.Serial(port=LORA_COM_PORT, baudrate=LORA_BAUDRATE)
    lora: LoRa = LoRa(lora_serial=lora_serial, timeout=LORA_TIMEOUT, num_retries=LORA_NUM_RETRIES, freq=LORA_FREQ, data_rate=LORA_DATA_RATE, bandwidth=LORA_BANDWIDTH, code_rate=LORA_CODE_RATE, tx_power=LORA_TX_POWER, lna=LORA_LNA, low_dr_opt=LORA_LOW_DR_OPT)
    print(f"LoRa begin: {lora.begin()}")
    print(f"LoRa RX: {lora.set_state(LoRaState.RX)}")
    # DATA: bytes = b"hello world"
    DATA: bytes = b"A" * 255
    print(f"Sending data: {lora.send(DATA)}")

    while True:
        if lora.recv():
            print(f"Data received: {lora.get_buffer().decode()}")
            time.sleep(0.4)
            print(f"Sending data: {lora.send(lora.get_buffer())}")