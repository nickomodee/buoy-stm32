from DataGenerator import DataGenerator
from ServerState import ServerState
from DataStreamParser import DataStreamParser, get_python_type
from CRC import crc32
from typing import Type, Literal
import pathlib
import sys

import logging
logger = logging.getLogger('[FirmwareUpdateHandler]')

EPSILON: float = 1e-5

def get_comparison_operator(a, b) -> str:
    if abs(a - b) <= EPSILON:
        return "=="
    if a < b:
        return "<"
    if a > b:
        return ">"
    return "[invalid comparison]"

class FirmwareUpdateHandler(DataGenerator):
    firmware_update_dir: Literal["firmware_update_files"] = "firmware_update_files"
    firmware_updater_dir_path: pathlib.Path = pathlib.Path(firmware_update_dir)
    current_firmware_version_filename: Literal["firmware_version.txt"] = "firmware_version.txt"
    current_firmware_version_file_path: pathlib.Path = firmware_updater_dir_path / current_firmware_version_filename
    current_firmware_filename: Literal["firmware.bin"] = "firmware.bin"
    current_firmware_file_path: pathlib.Path = firmware_updater_dir_path / current_firmware_filename

    buoy_firmware_update_dir: Literal["fw.bin"] = "fw.bin"
    buoy_firmware_update_size_dir: Literal["fw_size.bin"] = "fw_size.bin"
    buoy_firmware_update_checksum_dir: Literal["checksum.bin"] = "checksum.bin"
    buoy_firmware_update_available_dir: Literal["avail.txt"] = "avail.txt"
    buoy_update_is_available_indicator_byte: Literal[b"1"] = b"1"

    def __init__(self, server_state) -> None:
        self.server_state_: ServerState = server_state
    
    @staticmethod
    def parse_current_firmware_version_():
        firmware_version_type: Type = get_python_type(DataStreamParser.firmware_version_data_type)
        if not FirmwareUpdateHandler.current_firmware_version_file_path.exists():
            logger.debug(f"Firmware version file doesn't exist at {FirmwareUpdateHandler.current_firmware_version_file_path}")
            return firmware_version_type()
        try:
            with FirmwareUpdateHandler.current_firmware_version_file_path.open('r') as f:
                data: str = f.read()
                try:
                    current_firmware_version = firmware_version_type(data)
                    return current_firmware_version
                except ValueError as e:
                    logger.debug(f"Failed to parse firmware version from data: \"{data}\" from file at {FirmwareUpdateHandler.current_firmware_version_file_path} with error: {e}")
                    return firmware_version_type()
        except OSError as e:
            logger.debug(f"Failed to open firmware version file at {FirmwareUpdateHandler.current_firmware_version_file_path} with error: {e}")
            return firmware_version_type()
    
    @staticmethod
    def get_firmware_data_() -> bytes:
        if not FirmwareUpdateHandler.current_firmware_file_path.exists():
            logger.debug(f"Firmware file doesn't exist at {FirmwareUpdateHandler.current_firmware_file_path}")
            return b""
        try:
            with FirmwareUpdateHandler.current_firmware_file_path.open('rb') as f:
                data: bytes = f.read()
                return data
        except OSError as e:
            logger.debug(f"Failed to open firmware file at {FirmwareUpdateHandler.current_firmware_file_path} with error: {e}")
            return b""
        
    @staticmethod
    def should_we_update_(buoy_firmware_version, current_firmware_version):
        logger.debug(f"Buoy firmware version ({buoy_firmware_version}) {get_comparison_operator(buoy_firmware_version, current_firmware_version)} current firmware version ({current_firmware_version})")
        return (buoy_firmware_version < current_firmware_version) and (abs(buoy_firmware_version - current_firmware_version) > EPSILON)

    def generate_data(self) -> bytes:
        buoy_firmware_version = self.server_state_.get_firmware_version()
        current_firmware_version = self.parse_current_firmware_version_()
        firmware_data: bytes = b""
        if self.should_we_update_(buoy_firmware_version, current_firmware_version):
            firmware_data = self.get_firmware_data_()
        generated_data: bytes = self.generate_data_from_firmware_(firmware_data)
        return generated_data

    @staticmethod
    def calculate_firmware_size(firmware_data: bytes) -> int:
        return len(firmware_data)
    
    @staticmethod
    def calculate_firmware_checksum(firmware_data: bytes) -> int:
        crc32.reset()
        return crc32.update_bulk(firmware_data)

    @staticmethod
    def generate_data_from_firmware_(firmware_data: bytes) -> bytes: # we need to prepend the 4 bytes of the file size and the 2 bytes for the checksum
        firmware_size: int = FirmwareUpdateHandler.calculate_firmware_size(firmware_data)
        firmware_size_bytes: bytes = firmware_size.to_bytes(4, byteorder='little') # `uint32_t` in little endian format
        firmware_checksum: int = FirmwareUpdateHandler.calculate_firmware_checksum(firmware_data)
        firmware_checksum_bytes: bytes = firmware_checksum.to_bytes(4, byteorder='little') # `uint32_t` in little endian format
        output: bytes = firmware_size_bytes + firmware_checksum_bytes + firmware_data
        logger.debug(f"Firmware update size: {firmware_size}, Firmware update checksum: {firmware_checksum}")
        return output

    @staticmethod
    def copy_update(update_dir: str) -> None:
        firmware_data = FirmwareUpdateHandler.get_firmware_data_()
        firmware_size: int = FirmwareUpdateHandler.calculate_firmware_size(firmware_data)
        firmware_size_bytes: bytes = firmware_size.to_bytes(4, byteorder='little') # `uint32_t` in little endian format
        firmware_checksum: int = FirmwareUpdateHandler.calculate_firmware_checksum(firmware_data)
        firmware_checksum_bytes: bytes = firmware_checksum.to_bytes(4, byteorder='little') # `uint32_t` in little endian format
        
        update_path: pathlib.Path = pathlib.Path(update_dir)
        buoy_firmware_update_path: pathlib.Path = update_path / FirmwareUpdateHandler.buoy_firmware_update_dir
        buoy_firmware_update_size_path: pathlib.Path = update_path / FirmwareUpdateHandler.buoy_firmware_update_size_dir
        buoy_firmware_update_checksum_path: pathlib.Path = update_path / FirmwareUpdateHandler.buoy_firmware_update_checksum_dir
        buoy_firmware_update_available_path: pathlib.Path = update_path / FirmwareUpdateHandler.buoy_firmware_update_available_dir

        with buoy_firmware_update_path.open("wb") as f:
            f.write(firmware_data)
        with buoy_firmware_update_size_path.open("wb") as f:
            f.write(firmware_size_bytes)
        with buoy_firmware_update_checksum_path.open("wb") as f:
            f.write(firmware_checksum_bytes)
        with buoy_firmware_update_available_path.open("wb") as f:
            f.write(FirmwareUpdateHandler.buoy_update_is_available_indicator_byte)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Please provide a single argument with the update path")
        exit(-1)
    
    update_dir: str = sys.argv[1]
    FirmwareUpdateHandler.copy_update(update_dir)