from DataGenerator import DataGenerator

class DataConstructor(DataGenerator):
    def __init__(self, *generators) -> None:
        self.generators_: tuple[DataGenerator] = generators
    
    def generate_data(self) -> bytes:
        data: bytes = b""
        for generator in self.generators_:
            data += generator.generate_data()
        return data