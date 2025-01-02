from ServerState import ServerState
from DataStreamParser import DataStreamParser
from TimeGenerator import TimeGenerator
from FirmwareUpdateHandler import FirmwareUpdateHandler
from DataConstructor import DataConstructor
from Encryption import Encryption
from LoRa import LoRa, DataRate, Bandwidth, CodeRate, LNA, LowDrOpt
from BCP import BCP
import serial, serial.serialutil, serial.tools.list_ports
import sys
import time
from typing import Literal, List

import logging

LOGGER_LEVEL = logging.DEBUG

root = logging.getLogger()
root.setLevel(LOGGER_LEVEL)

handler = logging.StreamHandler(sys.stdout)
handler.setLevel(LOGGER_LEVEL)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
handler.setFormatter(formatter)
root.addHandler(handler)

server_state: ServerState = ServerState()
data_stream_parser: DataStreamParser = DataStreamParser(server_state)
time_generator: TimeGenerator = TimeGenerator()
firmware_update_handler: FirmwareUpdateHandler = FirmwareUpdateHandler(server_state)
data_constructor: DataConstructor = DataConstructor(time_generator, firmware_update_handler)

LORA_COM_PORT: Literal['/dev/ttyUSB0'] = '/dev/ttyUSB0'
LORA_BAUDRATE: Literal[9600] = 9600
# LoRa config
LORA_TIMEOUT: float = 10.0
LORA_NUM_RETRIES: Literal[1] = 1
LORA_FREQ: Literal[915000000] = 915000000  # 915 MHz
LORA_DATA_RATE: DataRate = DataRate.SF7
LORA_BANDWIDTH: Bandwidth = Bandwidth.BANDWIDTH_500_KHZ
LORA_CODE_RATE: CodeRate = CodeRate.RATE_4_BY_5
LORA_TX_POWER: Literal[22] = 22
LORA_LNA: LNA = LNA.ON
LORA_LOW_DR_OPT: LowDrOpt = LowDrOpt.AUTO

# BCP config
BCP_TIMEOUT: float = 17.0 # these should be different between the buoy and the server and ideally prime to avoid getting stuck
BCP_NUM_RETRIES: Literal[8] = 8 # `timeout * num_retries` should be similar between the buoy and the server

try:
    lora_serial: serial.Serial = serial.Serial(port=LORA_COM_PORT, baudrate=LORA_BAUDRATE)
except serial.serialutil.SerialException as e:
    root.error(f"Error opening serial communication on COM port '{LORA_COM_PORT}': {e}")
    root.error(f"Available ports: {list(map(lambda x: f'{x.device}: {x.description}', serial.tools.list_ports.comports()))}")
    exit(-1)
lora: LoRa = LoRa(lora_serial=lora_serial, timeout=LORA_TIMEOUT, num_retries=LORA_NUM_RETRIES, freq=LORA_FREQ, data_rate=LORA_DATA_RATE, bandwidth=LORA_BANDWIDTH, code_rate=LORA_CODE_RATE, tx_power=LORA_TX_POWER, lna=LORA_LNA, low_dr_opt=LORA_LOW_DR_OPT)

encryption_key: List[int] = [0x41] * 16
encryption: Encryption = Encryption(encryption_key)

bcp: BCP = BCP(timeout=BCP_TIMEOUT, num_retries=BCP_NUM_RETRIES, lora=lora, generate_data_func=data_constructor.generate_data, encryption=encryption)

def main() -> None:
    while True:
        try:
            lora_serial.close()
            lora_serial.open()
            while True:
                root.info("Listening...")
                root.info(f"BCP Message received: {bcp.listen(data_stream_parser.parse_data)!r}")
        except serial.serialutil.SerialException as e:
            root.error(f"Serial Error: {e}... Sleeping for 5 seconds")
            time.sleep(5)

if __name__ == "__main__":
    main()
