from abc import ABC, abstractmethod

class DataGenerator(ABC):
    @abstractmethod
    def generate_data(self) -> bytes:
        """Generate data in bytes to be sent to the buoy"""
        pass