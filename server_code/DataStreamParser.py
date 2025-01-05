from enum import Enum, auto
import numpy as np
from ServerState import ServerState, DataStreamState
from typing import Type

class DataType(Enum):
    BOOL = auto()
    UINT8_T = auto()
    UINT16_T = auto()
    HALF = auto()
    UINT32_T = auto()
    FLOAT = auto()
    UINT64_T = auto()
    DOUBLE = auto()

data_type_to_size_mapping: dict[DataType, int] = {DataType.BOOL: 1, DataType.UINT8_T: 1, DataType.UINT16_T: 2, DataType.HALF: 2, DataType.UINT32_T: 4, DataType.FLOAT: 4, DataType.UINT64_T: 8, DataType.DOUBLE: 8}
data_type_to_numpy_type_mapping: dict[DataType, int] = {DataType.BOOL: np.bool_, DataType.UINT8_T: np.uint8, DataType.UINT16_T: np.uint16, DataType.HALF: np.float16, DataType.UINT32_T: np.uint32, DataType.FLOAT: np.float32, DataType.UINT64_T: np.uint64, DataType.DOUBLE: np.float64}
data_type_to_python_type_mapping: dict[DataType, Type] = {DataType.BOOL: bool, DataType.UINT8_T: int, DataType.UINT16_T: int, DataType.HALF: float, DataType.UINT32_T: int, DataType.FLOAT: float, DataType.UINT64_T: int, DataType.DOUBLE: float}

def get_data_type_size(data_type: DataType) -> int: # in bytes
    return data_type_to_size_mapping[data_type]

def get_python_type(data_type: DataType) -> Type:
    return data_type_to_python_type_mapping[data_type]

def get_numpy_type(data_type: DataType) -> np.dtype:
    return data_type_to_numpy_type_mapping[data_type]

state_to_data_type_mapping: dict[DataStreamState, DataType] = {DataStreamState.IMU_STATUS: DataType.BOOL, DataStreamState.IMU_FILE_STATUS: DataType.BOOL, DataStreamState.IMU_COUNT: DataType.UINT32_T, DataStreamState.IMU_SLEEP_STATUS: DataType.BOOL, DataStreamState.FIRMWARE_VERSION: DataType.FLOAT, DataStreamState.SD_STATUS: DataType.BOOL, DataStreamState.RTC_STATUS: DataType.BOOL, DataStreamState.SECOND: DataType.UINT8_T, DataStreamState.MINUTE: DataType.UINT8_T, DataStreamState.HOUR: DataType.UINT8_T, DataStreamState.DAY: DataType.UINT8_T, DataStreamState.DAYOFWEEK: DataType.UINT8_T, DataStreamState.MONTH: DataType.UINT8_T, DataStreamState.YEAR: DataType.UINT16_T, DataStreamState.BATTERY_VOLTAGE_STATUS: DataType.BOOL, DataStreamState.BATTERY_VOLTAGE: DataType.FLOAT, DataStreamState.TEMP_SENSOR_COUNT: DataType.UINT8_T, DataStreamState.TEMP_SENSOR_STATUS: DataType.BOOL, DataStreamState.TEMP_SENSOR_DATA: DataType.FLOAT, DataStreamState.HUMIDITY_SENSOR_COUNT: DataType.UINT8_T, DataStreamState.HUMIDITY_SENSOR_STATUS: DataType.BOOL, DataStreamState.HUMIDITY_SENSOR_DATA: DataType.FLOAT, DataStreamState.PRESSURE_SENSOR_COUNT: DataType.UINT8_T, DataStreamState.PRESSURE_SENSOR_STATUS: DataType.BOOL, DataStreamState.PRESSURE_SENSOR_DATA: DataType.FLOAT, DataStreamState.UV_SENSOR_COUNT: DataType.UINT8_T, DataStreamState.UV_SENSOR_STATUS: DataType.BOOL, DataStreamState.UV_SENSOR_DATA: DataType.FLOAT, DataStreamState.VISIBLE_LIGHT_SENSOR_COUNT: DataType.UINT8_T, DataStreamState.VISIBLE_LIGHT_SENSOR_STATUS: DataType.BOOL, DataStreamState.VISIBLE_LIGHT_SENSOR_DATA: DataType.UINT16_T, DataStreamState.IR_LIGHT_SENSOR_COUNT: DataType.UINT8_T, DataStreamState.IR_LIGHT_SENSOR_STATUS: DataType.BOOL, DataStreamState.IR_LIGHT_SENSOR_DATA: DataType.UINT16_T, DataStreamState.IMU_ROTATION_I: DataType.HALF, DataStreamState.IMU_ROTATION_J: DataType.HALF, DataStreamState.IMU_ROTATION_K: DataType.HALF, DataStreamState.IMU_ROTATION_REAL: DataType.HALF, DataStreamState.IMU_ACCELERATION_X: DataType.HALF, DataStreamState.IMU_ACCELERATION_Y: DataType.HALF, DataStreamState.IMU_ACCELERATION_Z: DataType.HALF}

def get_state_data_type(state: DataStreamState) -> DataType:
    return state_to_data_type_mapping[state]

class DataStreamParser:
    imu_status_data_type: DataType = get_state_data_type(DataStreamState.IMU_STATUS)
    imu_file_status_data_type: DataType = get_state_data_type(DataStreamState.IMU_FILE_STATUS)
    imu_count_data_type: DataType = get_state_data_type(DataStreamState.IMU_COUNT)
    imu_sleep_status_data_type: DataType = get_state_data_type(DataStreamState.IMU_SLEEP_STATUS)
    firmware_version_data_type: DataType = get_state_data_type(DataStreamState.FIRMWARE_VERSION)
    sd_status_data_type: DataType = get_state_data_type(DataStreamState.SD_STATUS)
    rtc_status_data_type: DataType = get_state_data_type(DataStreamState.RTC_STATUS)
    second_data_type: DataType = get_state_data_type(DataStreamState.SECOND)
    minute_data_type: DataType = get_state_data_type(DataStreamState.MINUTE)
    hour_data_type: DataType = get_state_data_type(DataStreamState.HOUR)
    day_data_type: DataType = get_state_data_type(DataStreamState.DAY)
    day_of_week_data_type: DataType = get_state_data_type(DataStreamState.DAYOFWEEK)
    month_data_type: DataType = get_state_data_type(DataStreamState.MONTH)
    year_data_type: DataType = get_state_data_type(DataStreamState.YEAR)
    battery_voltage_status_data_type: DataType = get_state_data_type(DataStreamState.BATTERY_VOLTAGE_STATUS)
    battery_voltage_data_type: DataType = get_state_data_type(DataStreamState.BATTERY_VOLTAGE)
    temp_sensor_count_data_type: DataType = get_state_data_type(DataStreamState.TEMP_SENSOR_COUNT)
    temp_sensor_status_data_type: DataType = get_state_data_type(DataStreamState.TEMP_SENSOR_STATUS)
    temp_sensor_data_data_type: DataType = get_state_data_type(DataStreamState.TEMP_SENSOR_DATA)
    humidity_sensor_count_data_type: DataType = get_state_data_type(DataStreamState.HUMIDITY_SENSOR_COUNT)
    humidity_sensor_status_data_type: DataType = get_state_data_type(DataStreamState.HUMIDITY_SENSOR_STATUS)
    humidity_sensor_data_data_type: DataType = get_state_data_type(DataStreamState.HUMIDITY_SENSOR_DATA)
    pressure_sensor_count_data_type: DataType = get_state_data_type(DataStreamState.PRESSURE_SENSOR_COUNT)
    pressure_sensor_status_data_type: DataType = get_state_data_type(DataStreamState.PRESSURE_SENSOR_STATUS)
    pressure_sensor_data_data_type: DataType = get_state_data_type(DataStreamState.PRESSURE_SENSOR_DATA)
    uv_sensor_count_data_type: DataType = get_state_data_type(DataStreamState.UV_SENSOR_COUNT)
    uv_sensor_status_data_type: DataType = get_state_data_type(DataStreamState.UV_SENSOR_STATUS)
    uv_sensor_data_data_type: DataType = get_state_data_type(DataStreamState.UV_SENSOR_DATA)
    visible_light_sensor_count_data_type: DataType = get_state_data_type(DataStreamState.VISIBLE_LIGHT_SENSOR_COUNT)
    visible_light_sensor_status_data_type: DataType = get_state_data_type(DataStreamState.VISIBLE_LIGHT_SENSOR_STATUS)
    visible_light_sensor_data_data_type: DataType = get_state_data_type(DataStreamState.VISIBLE_LIGHT_SENSOR_DATA)
    ir_light_sensor_count_data_type: DataType = get_state_data_type(DataStreamState.IR_LIGHT_SENSOR_COUNT)
    ir_light_sensor_status_data_type: DataType = get_state_data_type(DataStreamState.IR_LIGHT_SENSOR_STATUS)
    ir_light_sensor_data_data_type: DataType = get_state_data_type(DataStreamState.IR_LIGHT_SENSOR_DATA)
    imu_rotation_i_data_type: DataType = get_state_data_type(DataStreamState.IMU_ROTATION_I)
    imu_rotation_j_data_type: DataType = get_state_data_type(DataStreamState.IMU_ROTATION_J)
    imu_rotation_k_data_type: DataType = get_state_data_type(DataStreamState.IMU_ROTATION_K)
    imu_rotation_real_data_type: DataType = get_state_data_type(DataStreamState.IMU_ROTATION_REAL)
    imu_acceleration_x_data_type: DataType = get_state_data_type(DataStreamState.IMU_ACCELERATION_X)
    imu_acceleration_y_data_type: DataType = get_state_data_type(DataStreamState.IMU_ACCELERATION_Y)
    imu_acceleration_z_data_type: DataType = get_state_data_type(DataStreamState.IMU_ACCELERATION_Z)

    def __init__(self, server_state: ServerState) -> None:
        self.server_state_: ServerState = server_state
        self.buffer_: bytes = b""
        self.bytes_needed_: int = 0
        self.state_: DataStreamState = DataStreamState.FIRMWARE_VERSION # DataStreamState.IMU_STATUS

        self.reset_()

    def reset_(self) -> None:
        self.buffer_ = b""
        self.state_ = DataStreamState.FIRMWARE_VERSION # DataStreamState.IMU_STATUS
        self.bytes_needed_ = get_data_type_size(self.imu_status_data_type)
    
    def parse_data(self, data: bytes, current_index: int, final_index: int) -> None:
        if current_index == 0:
            self.reset_() # in case the BCP transmission failed before `reset()` was called with `finish`

        processed: int = 0
        while processed < len(data):
            to_copy: int = min(len(data) - processed, self.bytes_needed_ - len(self.buffer_))
            
            self.buffer_ += data[processed:processed + to_copy]
            processed += to_copy

            if len(self.buffer_) == self.bytes_needed_:
                if self.handle_state_((processed == len(data)) and (current_index == final_index)):
                    return # if we are finished don't continue
        
        if current_index == final_index:
            self.reset_()

    @staticmethod
    def process_bytes_by_data_type(data: bytes, data_type: DataType): # all in little endian format
        python_type: Type = get_python_type(data_type)
        numpy_type: np.dtype = np.dtype(get_numpy_type(data_type))
        numpy_type = numpy_type.newbyteorder('<') # force little endian format
        return python_type(np.frombuffer(data, dtype=numpy_type))
    
    def handle_state_(self, finish: bool) -> bool:
        if len(self.buffer_) != self.bytes_needed_:
            return

        new_value = self.process_bytes_by_data_type(self.buffer_, get_state_data_type(self.state_))
        next_state: DataStreamState = self.server_state_.update_state(self.state_, new_value)
        if next_state == DataStreamState.FINISHED:
            self.reset_()
            return True
        self.state_ = next_state
        self.bytes_needed_ = get_data_type_size(get_state_data_type(next_state))
        
        self.buffer_ = b""

        if finish:
            self.reset_()

        return finish