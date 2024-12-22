from Encryption import Encryption
from CRC import crc16
from typing import Literal, Optional, List, Callable
from enum import Enum

import logging
logger = logging.getLogger('[Packet]')

MAX_PACKET_SIZE: Literal[246] = 246
PACKET_OVERHEAD: Literal[6] = 6  # the packet 'overhead' that isn't encrypted data
MAX_MESSAGE_SIZE: Literal[239] = 239
MESSAGE_OVERHEAD: Literal[8] = 8 # the message 'overhead' that isn't data
MAX_DATA_SIZE: int = MAX_MESSAGE_SIZE - MESSAGE_OVERHEAD

MESSAGE_TYPE_INDEX: Literal[0] = 0
MESSAGE_INDEX_1_INDEX: Literal[1] = 1
MESSAGE_INDEX_2_INDEX: Literal[2] = 2
MESSAGE_INDEX_3_INDEX: Literal[3] = 3
MESSAGE_INDEX_4_INDEX: Literal[4] = 4
MESSAGE_CHECKSUM_1_INDEX: Literal[5] = 5
MESSAGE_CHECKSUM_2_INDEX: Literal[6] = 6
MESSAGE_DATA_SIZE_INDEX: Literal[7] = 7
MESSAGE_DATA_START_INDEX: Literal[8] = 8

PACKET_SIZE_INDEX: Literal[0] = 0
PACKET_ILLEGAL_CHAR_INDEX: Literal[1] = 1
PACKET_IV_1_INDEX: Literal[2] = 2
PACKET_IV_2_INDEX: Literal[3] = 3
PACKET_CHECKSUM_1_INDEX: Literal[4] = 4
PACKET_CHECKSUM_2_INDEX: Literal[5] = 5
PACKET_ENCRYPTED_START_INDEX: Literal[6] = 6

class PacketType(Enum):
    SYN = 0
    SYNACK = 1
    ACK = 2
    DATA_DESC = 3
    DATA = 4
    FIN = 5

def print_hex(data: bytes, func: Callable[[object], None]) -> None:
    func(" ".join([f"0x{x:02x}" for x in data]))

def replace_byte_at_index(data: bytes, index: int, replacement: int) -> bytes:
    return data[:index] + replacement.to_bytes(1, byteorder='little') + data[index + 1:]

class Packet:
    _ILLEGAL_CHAR: int = ord('\r')

    def __init__(self, encryption: Encryption, type: Optional[PacketType]=None, index: Optional[int]=None, data: Optional[bytes]=None) -> None:
        self._encryption = encryption
        self._type: PacketType | None = None
        self._index: int = 0
        self._data: bytes = b""
        self._message_checksum: int = 0
        self._packet_checksum: int = 0
        self._iv: int = 0
        self._illegal_char_replacement: int = 0
        self._packet: bytes = b""
        if type is not None and index is not None and data is not None:
            self.set(type, index, data)
    
    def set(self, type: PacketType, index: int, data: bytes) -> None:
        self.set_type(type)
        self.set_index(index)
        self.set_data(data)

    def set_type(self, type: PacketType) -> None:
        self._type = type

    def get_type(self) -> PacketType:
        return self._type

    def set_index(self, index: int) -> None:
        if index == -1: # -1 is reserved for the BCP to tell it to keep the original index (if setting everything through the `set(PacketType, uint32_t, uint8_t, const char*)` method)
            return
        self._index = index

    def get_index(self) -> int:
        return self._index

    def set_message_checksum(self, checksum: int) -> None:
        self._message_checksum = checksum

    def get_message_checksum(self) -> int:
        return self._message_checksum

    def generate_message_checksum(self) -> int:
        crc16.reset()
        crc16.update(self._type.value)
        crc16.update_bulk(self._index.to_bytes(4, byteorder='little')) # little endian format
        # skip checksum for calculating the checksum
        crc16.update(len(self._data))
        DATA_SIZE: int = min(len(self._data), MAX_DATA_SIZE)
        crc16.update_bulk(self._data[:DATA_SIZE])
        return crc16.get_crc()

    def validate_message_checksum(self) -> bool:
        REAL_CHECKSUM: int = self.generate_message_checksum()
        return REAL_CHECKSUM == self._message_checksum
    
    def get_data_size(self) -> int:
        return len(self._data)

    def set_data(self, data: bytes) -> None:
        self._data = data[:MAX_DATA_SIZE]
    
    def get_data(self) -> bytes:
        return self._data

    def set_packet_checksum(self, checksum: int) -> None:
        self._packet_checksum = checksum

    def get_packet_checksum(self) -> int:
        return self._packet_checksum

    def generate_packet_checksum(self) -> int:
        crc16.reset()
        PACKET_SIZE: int = min(len(self._packet), MAX_PACKET_SIZE)
        for i in range(PACKET_SIZE):
            if (i == PACKET_CHECKSUM_1_INDEX) or (i == PACKET_CHECKSUM_2_INDEX) or (i == PACKET_ILLEGAL_CHAR_INDEX): # skip checksum and illegal char replacement (since if the checksum has an illegal char, we still want to handle this) for calculating the checksum
                continue
            crc16.update(self._packet[i])
        return crc16.get_crc()

    def validate_packet_checksum(self) -> bool:
        REAL_CHECKSUM: int = self.generate_packet_checksum()
        return REAL_CHECKSUM == self._packet_checksum

    def get_packet_size(self) -> int:
        return self._packet[PACKET_SIZE_INDEX]

    def set_packet(self, packet: bytes) -> None:
        self._packet = packet[:MAX_PACKET_SIZE]
    
    def get_packet(self) -> bytes:
        return self._packet

    def set_iv(self, iv: int) -> None:
        self._iv = iv

    def get_iv(self) -> int:
        return self._iv

    def set_illegal_char_replacement(self, illegal_char_replacement: int) -> None:
        self._illegal_char_replacement = illegal_char_replacement

    def get_illegal_char_replacement(self) -> int:
        return self._illegal_char_replacement

    def from_raw(self) -> bool:
        if len(self._packet) < PACKET_OVERHEAD:
            logger.debug(f"Error while parsing: Packet size is not large enough. Expected at least: {PACKET_OVERHEAD}, got: {len(self._packet)}")
            return False
        self.revert_illegal_char() # Do this FIRST THING
        if self._packet[PACKET_SIZE_INDEX] != len(self._packet):
            logger.debug(f"Error while parsing: Packet size not equal to expected packet size. Expected: {len(self._packet)}, got: {self._packet[PACKET_SIZE_INDEX]}")
            return False
        self.set_illegal_char_replacement(self._packet[PACKET_ILLEGAL_CHAR_INDEX])
        self.set_iv(self._packet[PACKET_IV_1_INDEX] | (self._packet[PACKET_IV_2_INDEX] << 8))
        self.set_packet_checksum(self._packet[PACKET_CHECKSUM_1_INDEX] | (self._packet[PACKET_CHECKSUM_2_INDEX] << 8))
        ENCRYPTED_DATA: bytes = self._packet[PACKET_OVERHEAD:]
        try:
            message: bytes = self._encryption.decrypt(ENCRYPTED_DATA, self._iv)
        except AssertionError as e:
            logger.debug(f"Error while decrypting: {e}")
            return False
        if (len(message) < MESSAGE_DATA_START_INDEX - 1) or (len(message) != MESSAGE_OVERHEAD + message[MESSAGE_DATA_SIZE_INDEX]):
            logger.debug(f"Error while decrypting message with invalid size: {len(message)}")
            return False
        self.set_type(PacketType(message[MESSAGE_TYPE_INDEX]))
        self.set_index(message[MESSAGE_INDEX_1_INDEX] | (message[MESSAGE_INDEX_2_INDEX] << 8) | (message[MESSAGE_INDEX_3_INDEX] << 16) | (message[MESSAGE_INDEX_4_INDEX] << 24))
        self.set_message_checksum(message[MESSAGE_CHECKSUM_1_INDEX] | (message[MESSAGE_CHECKSUM_2_INDEX] << 8))
        MESSAGE_DATA_SIZE: int = message[MESSAGE_DATA_SIZE_INDEX]
        self.set_data(message[MESSAGE_DATA_START_INDEX:MESSAGE_DATA_START_INDEX + MESSAGE_DATA_SIZE])
        return True

    def generate_raw(self) -> None:
        message: bytes = b""
        message += self._type.value.to_bytes(1, byteorder='little')
        message += self._index.to_bytes(4, byteorder='little') # little endian format
        MESSAGE_CHECKSUM: int = self.generate_message_checksum()
        self.set_message_checksum(MESSAGE_CHECKSUM)
        message += MESSAGE_CHECKSUM.to_bytes(2, byteorder='little') # little endian format
        message += len(self._data).to_bytes(1, byteorder='little')
        message += self._data[:MAX_DATA_SIZE]
        IV: int = self._encryption.generate_random_iv()
        self.set_iv(IV)
        encrypted_message: bytes = self._encryption.encrypt(message, IV)
        encrypted_message_bytearray: bytearray = bytearray(encrypted_message)
        encrypted_message = bytes(encrypted_message_bytearray)
        ENCRYPTED_SIZE: int = len(encrypted_message)
        PACKET_SIZE: int = ENCRYPTED_SIZE + PACKET_OVERHEAD
        self._packet = b""
        self._packet += PACKET_SIZE.to_bytes(1, byteorder='little')
        self._packet += Packet._ILLEGAL_CHAR.to_bytes(1, byteorder='little') # we need to set a TEMPORARY illegal char replacement so that we can generate the actual replacement below, after the IV, checksum, and encrypted message is set
        self._packet += IV.to_bytes(2, byteorder='little') # little endian format
        self._packet += b"0" # we need to set a TEMPORARY checksum so that we can generate the checksum below (but we first need to append the `encrypted_message` in the correct position)
        self._packet += b"0" # we need to set a TEMPORARY checksum so that we can generate the checksum below (but we first need to append the `encrypted_message` in the correct position)
        self._packet += encrypted_message
        PACKET_CHECKSUM: int = self.generate_packet_checksum()
        self.set_packet_checksum(PACKET_CHECKSUM)
        self._packet = replace_byte_at_index(self._packet, PACKET_CHECKSUM_1_INDEX, PACKET_CHECKSUM & 0xFF) # this is annoying maybe switch all bytes to bytearray to allow indexed assignment?
        self._packet = replace_byte_at_index(self._packet, PACKET_CHECKSUM_2_INDEX, PACKET_CHECKSUM >> 8) # this is annoying maybe switch all bytes to bytearray to allow indexed assignment?
        # We want to set the illegal char replacement last, in case IV or checksum also contains the illegal character
        ILLEGAL_CHAR_REPLACEMENT: int = self.replace_illegal_char()
        self.set_illegal_char_replacement(ILLEGAL_CHAR_REPLACEMENT)
        self._packet = replace_byte_at_index(self._packet, PACKET_ILLEGAL_CHAR_INDEX, ILLEGAL_CHAR_REPLACEMENT) # this is annoying maybe switch all bytes to bytearray to allow indexed assignment?

    def _find_missing_char(self) -> int:
        found = [False] * 256

        for index, char in enumerate(self._packet):
            if index == PACKET_ILLEGAL_CHAR_INDEX: # Don't consider the illegal char replacement
                continue

            found[char] = True

        for index, value in enumerate(found):
            if index != Packet._ILLEGAL_CHAR and not value:
                return index # return the missing character (that isn't the illegal char)

        # bad if we reach here...
        assert False, "Missing character not generated"
        # return self._ILLEGAL_CHAR

    def replace_illegal_char(self) -> int:
        MISSING_CHAR = self._find_missing_char()

        for index, char in enumerate(self._packet):
            if index == PACKET_ILLEGAL_CHAR_INDEX: # Don't replace the illegal char replacement
                continue

            if char == Packet._ILLEGAL_CHAR:
                self._packet = replace_byte_at_index(self._packet, index, MISSING_CHAR)

        return MISSING_CHAR

    def revert_illegal_char(self) -> None:
        # Unlike the microcontroller C++, we don't need to check if the packet size needs to be replaced immediately
        ILLEGAL_CHAR_REPLACEMENT = self._packet[PACKET_ILLEGAL_CHAR_INDEX]

        for index, char in enumerate(self._packet):
            if index == PACKET_ILLEGAL_CHAR_INDEX: # Don't replace the illegal char replacement
                continue

            if char == ILLEGAL_CHAR_REPLACEMENT:
                self._packet = replace_byte_at_index(self._packet, index, Packet._ILLEGAL_CHAR)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Packet):
            return NotImplemented

        return self._type == other.get_type() and self._index == other.get_index() and self._data == other.get_data()
    
    def __neq__(self, other: object) -> bool:
        if not isinstance(other, Packet):
            return NotImplemented

        return not (self == other)

if __name__ == "__main__":
    ENCRYPTION_KEY: List[int] = [0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41]
    encryption: Encryption = Encryption(ENCRYPTION_KEY)
    data: bytes = b"package this"
    packet_type: PacketType = PacketType.DATA
    packet_index: int = 1001
    packet: Packet = Packet(encryption, packet_type, packet_index, data)
    packet.generate_raw()
    packet_bytes: bytes = packet.get_packet()
    print_hex(packet_bytes, print)
    print(packet.get_type())
    print(packet.get_index())
    print(packet.get_message_checksum())
    print(packet.generate_message_checksum())
    print(packet.validate_message_checksum())
    print(packet.get_data_size())
    print(packet.get_packet_checksum())
    print(packet.generate_packet_checksum())
    print(packet.validate_packet_checksum())
    print(packet.get_packet_size())
    print(packet.get_iv())
    print(packet.get_illegal_char_replacement())

    decoded_packet: Packet = Packet(encryption)
    decoded_packet.set_packet(packet_bytes)
    # decoded_packet.set_packet(bytes([0x35, 0x02, 0x2e, 0x8f, 0x85, 0x47, 0x00, 0x6d, 0xa1, 0x89, 0x03, 0x57, 0x71, 0xa8, 0xcc, 0x22, 0x1e, 0xf4, 0x8a, 0x80, 0x61, 0xeb, 0x51, 0x01, 0x2b, 0xe0, 0xed, 0x32, 0x7d, 0x86, 0xaa, 0x88, 0x9e, 0xa3, 0xf4, 0x6a, 0x44, 0xe2, 0x11, 0x3e, 0x96, 0xda, 0x8a, 0x1a, 0x01, 0x1d, 0xaf, 0x88, 0x37, 0x37, 0x69, 0x35, 0xb2]))
    if not decoded_packet.from_raw():
        raise ValueError("Packet decode failed!")
    decoded_data: bytes = decoded_packet.get_data()
    print_hex(decoded_data, print)
    print(decoded_data.decode())
    print(decoded_packet.get_type())
    print(decoded_packet.get_index())
    print(decoded_packet.get_message_checksum())
    print(decoded_packet.generate_message_checksum())
    print(decoded_packet.validate_message_checksum())
    print(decoded_packet.get_data_size())
    print(decoded_packet.get_packet_checksum())
    print(decoded_packet.generate_packet_checksum())
    print(decoded_packet.validate_packet_checksum())
    print(decoded_packet.get_packet_size())
    print(decoded_packet.get_iv())
    print(decoded_packet.get_illegal_char_replacement())
    print_hex(decoded_packet.get_packet(), print)