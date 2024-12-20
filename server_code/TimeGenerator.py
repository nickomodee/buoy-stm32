from DataGenerator import DataGenerator
from datetime import datetime

class TimeGenerator(DataGenerator):
    def __init__(self) -> None:
        pass

    @staticmethod
    def generate_data() -> bytes:
        now: datetime = datetime.now()
        second: int = now.second
        minute: int = now.minute
        hour: int = now.hour
        day: int = now.day
        dayofweek: int = now.weekday() + 1 # `+ 1` to have the same format as the buoy, in section 27.6.2 on page 805 of the reference manual at: https://www.st.com/resource/en/reference_manual/dm00043574-stm32f303xb-c-d-e-stm32f303x6-8-stm32f328x8-stm32f358xc-stm32f398xe-advanced-arm-based-mcus-stmicroelectronics.pdf
        month: int = now.month
        year: int = now.year

        data: bytes = b""
        data += second.to_bytes(1, byteorder="little")
        data += minute.to_bytes(1, byteorder="little")
        data += hour.to_bytes(1, byteorder="little")
        data += day.to_bytes(1, byteorder="little")
        data += dayofweek.to_bytes(1, byteorder="little")
        data += month.to_bytes(1, byteorder="little")
        data += year.to_bytes(2, byteorder="little") # little endian format
        return data