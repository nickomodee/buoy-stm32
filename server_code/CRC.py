class CRC:
    CRC16_POLYNOMIAL: int = 0xF13B
    CRC32_POLYNOMIAL: int = 0x741B8CD7

    def __init__(self, data_type_size: int, polynomial: int) -> None:
        if data_type_size not in (8, 16, 32):
            raise ValueError("Unsupported data type size. Must be 8, 16, or 32 bits.")

        self.data_type_size: int = data_type_size
        self.polynomial: int = polynomial
        self.reset()

        self.width: int = data_type_size
        self.mask: int = (1 << self.width) - 1

    def reset(self) -> None:
        self.crc: int = 0

    def update(self, data: int) -> int:
        if not (0 <= data < (1 << 8)):
            raise ValueError(f"Data out of range for 8-bits.")

        self.crc = self.calculate_crc_(self.crc, data)
        return self.get_crc()

    def update_bulk(self, data_array: bytes) -> int:
        for data in data_array:
            self.update(data)
        return self.get_crc()

    def calculate_crc_(self, crc: int, data: int) -> int:
        for i in range(8):
            # Extract the current bit from the data
            bit: int = (data >> (7 - i)) & 1

            # Extract the top bit from the CRC
            c: int = ((crc >> (self.width - 1)) & 1) ^ bit

            # Shift CRC left by 1 and apply mask
            crc = ((crc << 1) & self.mask)

            # If XOR result is 1, XOR the polynomial
            if c:
                crc ^= self.polynomial

        return crc

    def get_crc(self) -> int:
        return self.crc & ((1 << self.data_type_size) - 1)

crc16 = CRC(16, CRC.CRC16_POLYNOMIAL)
crc32 = CRC(32, CRC.CRC32_POLYNOMIAL)

if __name__ == "__main__":
    data: bytes = b"ABCDEFGHIJKLMNOPQRSTUVWXYZ"

    crc16.reset()
    calculated_crc16 = crc16.update_bulk(data)
    print(f"Calculated CRC 16-bit: {calculated_crc16}")

    crc32.reset()
    calculated_crc32 = crc32.update_bulk(data)
    print(f"Calculated CRC 32-bit: {calculated_crc32}")