from Packet import Packet, PacketType, MAX_DATA_SIZE
from Encryption import Encryption
from LoRa import LoRa, LoRaState, DataRate, Bandwidth, CodeRate, IQConverted
import serial, serial.serialutil, serial.tools.list_ports
import time
from typing import Literal, List, Callable
from enum import Enum

import logging
logger = logging.getLogger(__name__)

BCP_SENT_HISTORY_SIZE: Literal[4] = 4
BCP_RECVD_HISTORY_SIZE: Literal[4] = 4

class RetransmissionController(Enum):
    BUOY = 0
    SERVER = 1

def print_hex(data: bytes, func: Callable[[object], None]) -> None:
    func(" ".join([f"0x{x:02x}" for x in data]))

class BCP:
    def __init__(self, timeout: float, num_retries: int, lora: LoRa, encryption: Encryption) -> None:
        self._timeout: float = timeout
        self._num_retries: int = num_retries
        self._lora: LoRa = lora
        self.__current_index: int = 0
        self._sent_packet_history: List[Packet] = [Packet(encryption) for i in range(BCP_SENT_HISTORY_SIZE)]
        self._recvd_packet_history: List[Packet] = [Packet(encryption) for i in range(BCP_RECVD_HISTORY_SIZE)]
        self._total_msg_packets: int = 0
        self._encryption: Encryption = encryption
        self._recv_buffer: bytes = b""
        self._retransmission_control: RetransmissionController = RetransmissionController.SERVER

    @property
    def _current_index(self) -> int:
        # logger.debug("Getting _current_index value")
        return self.__current_index

    @_current_index.setter
    def _current_index(self, value: int) -> None:
        # logger.debug(f"Setting _current_index to: {value}")
        self.__current_index = value

    def listen(self, data: bytes) -> bytes:
        while True:
            saved_state: LoRaState = self._lora.get_state()
            self._current_index = 0
            self._recv_buffer = b""
            self._sent_packet_history = [Packet(encryption) for i in range(BCP_SENT_HISTORY_SIZE)]
            self._recvd_packet_history = [Packet(encryption) for i in range(BCP_RECVD_HISTORY_SIZE)]
            self._lora.set_state(LoRaState.RX)
            self._set_retransmission_control(RetransmissionController.BUOY)
            if self._handle_recv(PacketType.SYN):
                # begin
                start_time: float = time.time()
                logger.info("BCP message begin")
                logger.info("Synchronising")
                if not self._synchronise():
                    continue
                logger.info("Receiving DATA_DESC")
                if not self._recv_data_desc():
                    continue
                logger.info("Receiving data")
                if not self._recv_data():
                    continue
                self._set_retransmission_control(RetransmissionController.SERVER)
                logger.info("Sending DATA_DESC")
                if not self._send_data_desc(len(data)):
                    continue
                logger.info("Sending data")
                if not self._send_data(data):
                    continue
                self._set_retransmission_control(RetransmissionController.BUOY)
                logger.debug("Finishing")
                if not self._finish():
                    continue
                logger.info("Finished")
                self._lora.set_state(saved_state)
                time_difference: float = time.time() - start_time
                logger.info(f"BCP message finished in {time_difference // 60:.0f}:{time_difference % 60:05.2f}")
                return self._recv_buffer

    def _send_packet(self, history_index: int) -> bool:
        if history_index >= BCP_SENT_HISTORY_SIZE:
            return False
        
        packet: Packet = self._sent_packet_history[history_index]
        packet.generate_raw()
        packet_data: bytes = packet.get_packet()
        self._current_index = packet.get_index() + 1 # in some cases on retransmission this is necessary
        logger.debug(f"Sending: {packet.get_type()}, Index: {packet.get_index()}")
        print_hex(packet.get_packet(), logger.debug)
        print_hex(packet.get_data(), logger.debug)
        return self._lora.send(packet_data)

    def _new_send_packet(self) -> Packet:
        for i in range(BCP_SENT_HISTORY_SIZE - 1, 0, -1): # we don't want to include 0
            self._sent_packet_history[i] = self._sent_packet_history[i - 1]
        
        self._sent_packet_history[0] = Packet(self._encryption)
        self._sent_packet_history[0].set_index(self._current_index)
        self._current_index += 1
        return self._sent_packet_history[0]

    def _new_recv_packet(self) -> Packet:
        for i in range(BCP_RECVD_HISTORY_SIZE - 1, 0, -1): # we don't want to include 0
            self._recvd_packet_history[i] = self._recvd_packet_history[i - 1]
        
        self._recvd_packet_history[0] = Packet(self._encryption)
        self._current_index += 1
        return self._recvd_packet_history[0]

    def _send_recv(self, num_packets: int, expected_packet_type: PacketType) -> bool:
        if num_packets > BCP_SENT_HISTORY_SIZE:
            return False

        for i in range(self._num_retries):
            send_failed: bool = False
            for packet_index in range(num_packets - 1, -1, -1):
                if not self._send_packet(packet_index):
                    send_failed = True
                    break
            if send_failed:
                continue

            start_time: float = time.time()
            while time.time() - start_time < self._timeout:
                if self._recv_packet():
                    if self._recvd_packet_history[0].get_type() != expected_packet_type:
                        self._reject_recvd_packet()
                        continue
                    return True # success
        return False

    def _handle_recv(self, expected_packet_type: PacketType) -> bool:
        start_time: float = time.time()
        while time.time() - start_time < self._num_retries * self._timeout: # transmission control is up to the server, so timeout should be multiplied by num of retries to allow buoy to send packet in case of packet loss            
            if self._recv_packet():
                if self._recvd_packet_history[0].get_type() == expected_packet_type:
                    return True
                
                if self._recvd_packet_history[0] == self._recvd_packet_history[1]: # if this is a retransmission just resend last sent packet, but we don't break, as we need to keep checking for the expected next packet
                    self._send_packet(0) # doesn't matter if this fails
                    continue

                self._reject_recvd_packet() # else, reject
                
        return False

    def _synchronise(self) -> bool:
        # SYN has already been received
        new_packet: Packet = self._new_send_packet()
        new_packet.set(PacketType.SYNACK, -1, b"")
        if not self._send_packet(0):
            return False
        return self._handle_recv(PacketType.ACK)

    def _recv_data_desc(self) -> bool:
        start_time: float = time.time()
        while time.time() - start_time < self._num_retries * self._timeout: # transmission control is up to the buoy, so timeout should be multiplied by num of retries to allow buoy to send packet in case of packet loss
            if self._recv_packet():
                if self._recvd_packet_history[0].get_type() != PacketType.DATA_DESC:
                    self._reject_recvd_packet()
                    continue
                self._total_msg_packets = int.from_bytes(self._recvd_packet_history[0].get_data(), byteorder='little') # little endian format
                logger.debug(f"Recv DATA_DESC, Num packets: {self._total_msg_packets}")

                new_packet: Packet = self._new_send_packet()
                new_packet.set(PacketType.ACK, -1, b"")
                return self._send_packet(0)
        return False

    def _recv_data(self) -> bool:
        for i in range(self._total_msg_packets):
            timed_out: bool = True
            start_time: float = time.time()
            while time.time() - start_time < self._num_retries * self._timeout:
                if self._recv_packet():
                    if self._recvd_packet_history[0].get_type() == PacketType.DATA:
                        self._recv_buffer += self._recvd_packet_history[0].get_data()

                        new_packet: Packet = self._new_send_packet()
                        new_packet.set(PacketType.ACK, -1, b"")

                        if i == self._total_msg_packets - 1: # if this is the last data packet, don't send the ACK, as we will do this in `send_data_desc()`
                            return True
                        if not self._send_packet(0):
                            continue

                        timed_out = False
                        break

                    if self._recvd_packet_history[0] == self._recvd_packet_history[1]: # if this is a retransmission just resend last sent packet, but we don't break, as we need to keep checking for the expected next packet
                        self._send_packet(0)
                        continue

                    self._reject_recvd_packet() # else, reject
            if timed_out:
                return False
        return True

    def _send_data_desc(self, data_size: int) -> bool:
        new_packet: Packet = self._new_send_packet()
        num_packets: int = int(data_size / MAX_DATA_SIZE) + (1 if data_size % MAX_DATA_SIZE else 0)
        logger.debug(f"Send DATA_DESC, Num packets: {self._total_msg_packets}")
        new_packet.set(PacketType.DATA_DESC, -1, num_packets.to_bytes(4, byteorder='little')) # little endian format
        return self._send_recv(2, PacketType.ACK) # send the ACK from `recv_data()` first, and then the DATA_DESC

    def _send_data(self, data: bytes) -> bool:
        data_size: int = len(data)
        num_packets: int = int(data_size / MAX_DATA_SIZE) + (1 if data_size % MAX_DATA_SIZE else 0)
        for i in range(num_packets):
            new_packet: Packet = self._new_send_packet()
            new_packet.set(PacketType.DATA, -1, data[i * MAX_DATA_SIZE:i * MAX_DATA_SIZE + MAX_DATA_SIZE])
            if not self._send_recv(1, PacketType.ACK):
                return False
        return True

    def _finish(self) -> bool:
        if not self._handle_recv(PacketType.FIN):
            return False
        new_packet: Packet = self._new_send_packet()
        new_packet.set(PacketType.ACK, -1, b"")
        new_packet = self._new_send_packet()
        new_packet.set(PacketType.FIN, -1, b"")
        return self._send_recv(2, PacketType.ACK)

    def _recv_packet(self) -> bool:
        start_time: float = time.time()
        while time.time() - start_time < self._timeout:
            if self._lora.recv():
                data: bytes = self._lora.get_buffer()
                new_packet: Packet = self._new_recv_packet()
                new_packet.set_packet(data)
                if not new_packet.from_raw():
                    logger.debug("Packet invalidated, error while decoding raw bytes")
                    self._reject_recvd_packet()
                    continue
                logger.debug(f"Recvd packet: {new_packet.get_type()}, Index: {new_packet.get_index()}")
                if not self._validate_recvd_packet():
                    self._reject_recvd_packet()
                    return False
                return True
        return False

    def _validate_recvd_packet(self) -> bool:
        if not self._recvd_packet_history[0].validate_message_checksum() or not self._recvd_packet_history[0].validate_packet_checksum():
            logger.debug(f"Packet invalidated, Checksum failed Message checksum success: {self._recvd_packet_history[0].validate_message_checksum()} Packet checksum success: {self._recvd_packet_history[0].validate_packet_checksum()}")
            return False
        
        if self._recvd_packet_history[0].get_index() == self._current_index - 1:
            return True
        
        if self._retransmission_control == RetransmissionController.BUOY and self._recvd_packet_history[0] == self._recvd_packet_history[1] and self._recvd_packet_history[0].get_iv() != self._recvd_packet_history[1].get_iv(): # probably a duplicate if the IVs are equal. if buoy controls retransmission, it may miss our packet, so we may receive an already received packet
            logger.debug("Recvd retransmission packet")
            self._current_index = self._recvd_packet_history[0].get_index() + 1
            return True
        logger.debug(f"Packet invalidated, Current index: {self._current_index} Packet index: {self._recvd_packet_history[0].get_index()} Other packet index: {self._recvd_packet_history[1].get_index()} This packet type: {self._recvd_packet_history[0].get_type()} Other packet type: {self._recvd_packet_history[1].get_type()} This packet data: {self._recvd_packet_history[0].get_data()} Other packet data: {self._recvd_packet_history[1].get_data()}")
        return False

    def _reject_recvd_packet(self) -> None:
        for i in range(BCP_RECVD_HISTORY_SIZE - 1):
            self._recvd_packet_history[i] = self._recvd_packet_history[i + 1]
        self._recvd_packet_history[BCP_RECVD_HISTORY_SIZE - 1] = Packet(self._encryption)
        self._current_index -= 1
        logger.debug("Packet rejected")

    def _set_retransmission_control(self, controller: RetransmissionController) -> None:
        self._retransmission_control = controller

if __name__ == "__main__":
    import logging
    import sys

    LOGGER_LEVEL = logging.DEBUG

    root = logging.getLogger()
    root.setLevel(LOGGER_LEVEL)

    handler = logging.StreamHandler(sys.stdout)
    handler.setLevel(LOGGER_LEVEL)
    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    handler.setFormatter(formatter)
    root.addHandler(handler)

    LORA_COM_PORT: Literal['COM5'] = 'COM5'
    LORA_BAUDRATE: Literal[9600] = 9600
    # LoRa config
    LORA_TIMEOUT: float = 5.0
    LORA_NUM_RETRIES: Literal[5] = 5
    LORA_LOCAL_ADDR: Literal[102] = 102 # Swapped from the microcontroller end
    LORA_TARGET_ADDR: Literal[101] = 101
    LORA_FREQ: Literal[915000000] = 915000000  # 915 MHz
    LORA_DATA_RATE: DataRate = DataRate.SF12
    LORA_BANDWIDTH: Bandwidth = Bandwidth.BANDWIDTH_250_KHZ
    LORA_CODE_RATE: CodeRate = CodeRate.RATE_4_BY_5
    LORA_TX_POWER: Literal[22] = 22
    LORA_IQCONVERTED: IQConverted = IQConverted.OFF

    # BCP config
    BCP_TIMEOUT: float = 10.0
    BCP_NUM_RETRIES: Literal[5] = 5

    try:
        lora_serial: serial.Serial = serial.Serial(port=LORA_COM_PORT, baudrate=LORA_BAUDRATE)
    except serial.serialutil.SerialException as e:
        print(f"Error opening serial communication on COM port '{LORA_COM_PORT}': {e}")
        print(f"Available ports: {list(map(lambda x: f'{x.device}: {x.description}', serial.tools.list_ports.comports()))}")
        exit(-1)
    lora: LoRa = LoRa(lora_serial=lora_serial, timeout=LORA_TIMEOUT, num_retries=LORA_NUM_RETRIES, local_addr=LORA_LOCAL_ADDR, target_addr=LORA_TARGET_ADDR, freq=LORA_FREQ, data_rate=LORA_DATA_RATE, bandwidth=LORA_BANDWIDTH, code_rate=LORA_CODE_RATE, tx_power=LORA_TX_POWER, iqconverted=LORA_IQCONVERTED)
    
    encryption_key: List[int] = [0x41] * 16
    encryption: Encryption = Encryption(encryption_key)

    while True:
        try:
            lora_serial.close()
            lora_serial.open()
            lora_begin_status: bool = lora.begin()
            print(f"LoRa begin: {lora_begin_status}")
            if not lora_begin_status:
                time.sleep(5)
                continue

            while True:
                bcp_instance: BCP = BCP(timeout=BCP_TIMEOUT, num_retries=BCP_NUM_RETRIES, lora=lora, encryption=encryption)
                data: bytes = b"A" * 32768
                print(f"BCP Message received: {bcp_instance.listen(data)}")
        except serial.serialutil.SerialException as e:
            print(f"Serial Error: {e}")