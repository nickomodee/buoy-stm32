import serial
import time
from typing import Literal, List
from enum import Enum

LORA_MAX_SIZE = 61

class DataRate(Enum):
    SF12 = 0
    SF11 = 1
    SF10 = 2
    SF9 = 3
    SF8 = 4
    SF7 = 5
    SF6 = 6
    SF5 = 7

class Bandwidth(Enum):
    BANDWIDTH_125_KHZ = 0
    BANDWIDTH_250_KHZ = 1
    BANDWIDTH_500_KHZ = 2
    BANDWIDTH_62_5_KHZ = 3
    BANDWIDTH_41_67_KHZ = 4
    BANDWIDTH_31_25_KHZ = 5
    BANDWIDTH_20_83_KHZ = 6
    BANDWIDTH_15_63_KHZ = 7
    BANDWIDTH_10_42_KHZ = 8
    BANDWIDTH_7_81_KHZ = 9

class CodeRate(Enum):
    RATE_4_BY_5 = 1
    RATE_4_BY_6 = 2
    RATE_4_BY_7 = 3
    RATE_4_BY_8 = 4

class IQConverted(Enum):
    OFF = 0
    ON = 1

class LoRaState(Enum):
    RX = 0
    TX = 1
    IDLE = 2
    SLEEP = 3

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
    def __init__(self, lora_serial: serial.Serial, timeout: float, num_retries: int, local_addr: int, target_addr: int, freq: int, data_rate: DataRate, bandwidth: Bandwidth, code_rate: CodeRate, tx_power: int, iqconverted: IQConverted) -> None:
        self._lora_serial: serial.Serial = lora_serial
        self._serial_buffer: bytes = b""
        self._timeout: float = timeout
        self._num_retries: int = num_retries
        self._local_addr: int = local_addr
        self._target_addr: int = target_addr
        self._freq: int = freq
        self._data_rate: DataRate = data_rate
        self._bandwidth: Bandwidth = bandwidth
        self._code_rate: CodeRate = code_rate
        self._tx_power: int = tx_power
        self._iqconverted: IQConverted = iqconverted
        self._state: LoRaState = LoRaState.IDLE
        self._buffer: bytes = b""

    def begin(self) -> bool:
        for i in range(self._num_retries):
            self._lora_serial.write(b"+++\r\n")
        self._flush_serial()
        if not self._set_local_addr(self._local_addr):
            return False
        return self._set_target_addr(self._target_addr)

    def send(self, data: bytes | List[int]) -> bool:
        if len(data) > LORA_MAX_SIZE:
            return False
        
        SAVED_STATE: LoRaState = self._state
        if not self.set_state(LoRaState.TX):
            self.set_state(SAVED_STATE)
            return False
        self._buffer = bytes(data)
        self._buffer += b"\r\n"
        EXPECTED_BUFER: bytes = b"OnTxDone\r\n"
        if not self._send_command(self._buffer, EXPECTED_BUFER):
            self.set_state(SAVED_STATE)
            return False
        self.set_state(SAVED_STATE)
        return True

    def set_state(self, updated_state: LoRaState) -> bool:
        if self._state == updated_state:
            return True
        
        if self._state == LoRaState.TX:
            command_buffer: bytes = b"+++\r\n"
            expected_buffer: bytes = b"Quit transparent\r\n"
            if not self._send_command(command_buffer, expected_buffer):
                return False
            # success exiting TX mode
        
        if self._state == LoRaState.SLEEP:
            command_buffer = b"\r\n"
            expected_buffer = b"leave deepsleep...\r\n"
            if not self._send_command(command_buffer, expected_buffer):
                return False
            # after exiting deep sleep, the next command will always error, sò we force an error to not disrupt any future commands
            EXPECTED_BUFFER_2: bytes = b"+CME ERROR:1\r\n"
            if not self._send_command(command_buffer, EXPECTED_BUFFER_2):
                return False
        
        self._state = LoRaState.IDLE

        match updated_state:
            case LoRaState.RX:
                command_buffer = f"AT+CRX={self._freq},{self._data_rate.value},{self._bandwidth.value},{self._code_rate.value},{self._iqconverted.value}\r\n".encode()
                expected_buffer = b")\r\n"
                if not self._send_command(command_buffer, expected_buffer):
                    return False
                self._state = LoRaState.RX
                return True
            case LoRaState.TX:
                command_buffer = f"AT+CTX={self._freq},{self._data_rate.value},{self._bandwidth.value},{self._code_rate.value},{self._tx_power},{self._iqconverted.value}\r\n".encode()
                expected_buffer = b"\r\n>"
                if not self._send_command(command_buffer, expected_buffer):
                    return False
                self._state = LoRaState.TX
                return True
            case LoRaState.IDLE:
                return True
            case LoRaState.SLEEP:
                command_buffer = b"AT+CSLEEP=0\r\n" # constant 0 for hot-start
                expected_buffer = b"ener deepsleep...\r\n"
                if not self._send_command(command_buffer, expected_buffer):
                    return False
                self._state = LoRaState.SLEEP
                return True
            case _:
                return False

    def get_state(self) -> LoRaState:
        return self._state

    def wake(self) -> bool:
        if self._state != LoRaState.SLEEP:
            return True
        
        return self.set_state(LoRaState.IDLE)

    def recv(self) -> bool:
        SAVED_STATE: LoRaState = self._state
        if not self.set_state(LoRaState.RX):
            self.set_state(SAVED_STATE)
            return False
        for i in range(self._num_retries):
            self._buffer = b""
            expected_buffer: bytes = b"Recv:\r\n"
            if not self._expected(expected_buffer):
                continue
            c1: int = self._read_blocking()
            c2: int = self._read_blocking()
            while c1 != ord('\r'):
                if len(self._buffer) >= LORA_MAX_SIZE:
                    self.set_state(SAVED_STATE)
                    return False
                self._buffer += ((hex_char_to_byte(c1) << 4) | hex_char_to_byte(c2)).to_bytes(1, byteorder='big')
                self._read_blocking() # flush ' ' char
                c1 = self._read_blocking()
                c2 = self._read_blocking()
            expected_buffer = b"\r\n"
            # we need 2 more "\r\n" for RX finish
            if not self._expected(expected_buffer):
                self.set_state(SAVED_STATE)
                return False
            if not self._expected(expected_buffer):
                self.set_state(SAVED_STATE)
                return False
            self.set_state(SAVED_STATE)
            return True
        self.set_state(SAVED_STATE)
        return False

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

    def _set_local_addr(self, local_addr: int) -> bool:
        COMMAND_BUFFER: bytes = f"AT+CADDRSET={local_addr}\r\n".encode()
        EXPECTED_BUFFER: bytes = f"set local address: {local_addr} \r\n".encode() # yes, the space after is not a typo
        if not self._send_command(COMMAND_BUFFER, EXPECTED_BUFFER):
            return False
        # success
        self._local_addr = local_addr
        return True

    def _set_target_addr(self, target_addr: int) -> bool:
        COMMAND_BUFFER: bytes = f"AT+CTXADDRSET={target_addr}\r\n".encode()
        EXPECTED_BUFFER: bytes = f"set target address: {target_addr} \r\n".encode() # yes, the space after is not a typo
        if not self._send_command(COMMAND_BUFFER, EXPECTED_BUFFER):
            return False
        # success
        self._target_addr = target_addr
        return True

    def _expected(self, expected: bytes) -> bool:
        START_TIME: float = time.time()
        result: bytes = b""
        while len(result) < len(expected) or result[-len(expected):] != expected:
            # check if timeout reached
            if time.time() - START_TIME >= self._timeout:
                return False
            
            # check if byte available to read (so that we don't block)
            if self._lora_serial.in_waiting:
                result += self._lora_serial.read(1)
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
    LORA_COM_PORT: Literal['COM18'] = 'COM18'
    LORA_BAUDRATE: Literal[9600] = 9600
    # LoRa config
    LORA_TIMEOUT: float = 1.0
    LORA_NUM_RETRIES: Literal[5] = 5
    LORA_LOCAL_ADDR: Literal[102] = 102
    LORA_TARGET_ADDR: Literal[101] = 101
    LORA_FREQ: Literal[920000000] = 920000000  # 920 MHz
    LORA_DATA_RATE: DataRate = DataRate.SF7
    LORA_BANDWIDTH: Bandwidth = Bandwidth.BANDWIDTH_125_KHZ
    LORA_CODE_RATE: CodeRate = CodeRate.RATE_4_BY_5
    LORA_TX_POWER: Literal[0] = 0
    LORA_IQCONVERTED: IQConverted = IQConverted.OFF

    lora_serial: serial.Serial = serial.Serial(port=LORA_COM_PORT, baudrate=LORA_BAUDRATE)
    lora: LoRa = LoRa(lora_serial=lora_serial, timeout=LORA_TIMEOUT, num_retries=LORA_NUM_RETRIES, local_addr=LORA_LOCAL_ADDR, target_addr=LORA_TARGET_ADDR, freq=LORA_FREQ, data_rate=LORA_DATA_RATE, bandwidth=LORA_BANDWIDTH, code_rate=LORA_CODE_RATE, tx_power=LORA_TX_POWER, iqconverted=LORA_IQCONVERTED)
    print(f"LoRa begin: {lora.begin()}")
    print(f"LoRa RX: {lora.set_state(LoRaState.RX)}")
    DATA: bytes = b"hello world"
    print(f"Sending data: {lora.send(DATA)}")

    while True:
        if lora.recv():
            print(f"Data received: {lora.get_buffer().decode()}")
            time.sleep(0.4)
            print(f"Sending data: {lora.send(lora.get_buffer())}")