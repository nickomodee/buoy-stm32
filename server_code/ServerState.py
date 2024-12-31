from enum import Enum, auto
from Send import send_data_to_backend
import threading
from typing import List, Literal
import json

import logging
logger = logging.getLogger("[ServerState]")

IMU_LOG_FILE: Literal["imu_log.log"] = "imu_log.log"

class DataStreamState(Enum):
    IMU_STATUS = auto()
    IMU_FILE_STATUS = auto()
    IMU_COUNT = auto()
    IMU_SLEEP_STATUS = auto()
    FIRMWARE_VERSION = auto()
    SD_STATUS = auto()
    RTC_STATUS = auto()
    SECOND = auto()
    MINUTE = auto()
    HOUR = auto()
    DAY = auto()
    DAYOFWEEK = auto()
    MONTH = auto()
    YEAR = auto()
    BATTERY_VOLTAGE_STATUS = auto()
    BATTERY_VOLTAGE = auto()
    TEMP_SENSOR_COUNT = auto()
    TEMP_SENSOR_STATUS = auto()
    TEMP_SENSOR_DATA = auto()
    HUMIDITY_SENSOR_COUNT = auto()
    HUMIDITY_SENSOR_STATUS = auto()
    HUMIDITY_SENSOR_DATA = auto()
    PRESSURE_SENSOR_COUNT = auto()
    PRESSURE_SENSOR_STATUS = auto()
    PRESSURE_SENSOR_DATA = auto()
    UV_SENSOR_COUNT = auto()
    UV_SENSOR_STATUS = auto()
    UV_SENSOR_DATA = auto()
    VISIBLE_LIGHT_SENSOR_COUNT = auto()
    VISIBLE_LIGHT_SENSOR_STATUS = auto()
    VISIBLE_LIGHT_SENSOR_DATA = auto()
    IR_LIGHT_SENSOR_COUNT = auto()
    IR_LIGHT_SENSOR_STATUS = auto()
    IR_LIGHT_SENSOR_DATA = auto()
    IMU_ROTATION_I = auto()
    IMU_ROTATION_J = auto()
    IMU_ROTATION_K = auto()
    IMU_ROTATION_REAL = auto()
    IMU_ACCELERATION_X = auto()
    IMU_ACCELERATION_Y = auto()
    IMU_ACCELERATION_Z = auto()
    FINISHED = auto() # not sent by buoy this is for server processing

class ServerState:
    def __init__(self) -> None:
        self.state_dict_: dict = {}
        self.imu_status_ = False
        self.imu_file_status_ = False
        self.imu_count_ = 0
        self.imu_sleep_status_ = False
        self.firmware_version_ = 0.0
        self.sd_status_ = False
        self.rtc_status_ = False
        self.second_ = 0
        self.minute_ = 0
        self.hour_ = 0
        self.day_ = 0
        self.day_of_week_ = 0
        self.month_ = 0
        self.year_ = 0
        self.battery_voltage_status_ = False
        self.battery_voltage_ = 0.0
        self.temp_sensor_count_ = 0
        self.temp_sensors_status_: List = []
        self.temp_sensors_data_: List = []
        self.humidity_sensor_count_ = 0
        self.humidity_sensors_status_: List = []
        self.humidity_sensors_data_: List = []
        self.pressure_sensor_count_ = 0
        self.pressure_sensors_status_: List = []
        self.pressure_sensors_data_: List = []
        self.uv_sensor_count_ = 0
        self.uv_sensors_status_: List = []
        self.uv_sensors_data_: List = []
        self.visible_light_sensor_count_ = 0
        self.visible_light_sensors_status_: List = []
        self.visible_light_sensors_data_: List = []
        self.ir_light_sensor_count_ = 0
        self.ir_light_sensors_status_: List = []
        self.ir_light_sensors_data_: List = []
        self.imu_rotation_i_ = []
        self.imu_rotation_j_ = []
        self.imu_rotation_k_ = []
        self.imu_rotation_real_ = []
        self.imu_acceleration_x_ = []
        self.imu_acceleration_y_ = []
        self.imu_acceleration_z_ = []

        self.reset_()
    
    def reset_(self):
        self.state_dict_ = {}
        self.imu_status_ = False
        self.imu_file_status_ = False
        self.imu_count_ = 0
        self.imu_sleep_status_ = False
        self.firmware_version_ = 0.0
        self.sd_status_ = False
        self.rtc_status_ = False
        self.second_ = 0
        self.minute_ = 0
        self.hour_ = 0
        self.day_ = 0
        self.day_of_week_ = 0
        self.month_ = 0
        self.year_ = 0
        self.battery_voltage_status_ = False
        self.battery_voltage_ = 0.0
        self.temp_sensor_count_ = 0
        self.temp_sensors_status_ = []
        self.temp_sensors_data_ = []
        self.humidity_sensor_count_ = 0
        self.humidity_sensors_status_ = []
        self.humidity_sensors_data_ = []
        self.pressure_sensor_count_ = 0
        self.pressure_sensors_status_ = []
        self.pressure_sensors_data_ = []
        self.uv_sensor_count_ = 0
        self.uv_sensors_status_ = []
        self.uv_sensors_data_ = []
        self.visible_light_sensor_count_ = 0
        self.visible_light_sensors_status_ = []
        self.visible_light_sensors_data_ = []
        self.ir_light_sensor_count_ = 0
        self.ir_light_sensors_status_ = []
        self.ir_light_sensors_data_ = []
        self.imu_rotation_i_ = []
        self.imu_rotation_j_ = []
        self.imu_rotation_k_ = []
        self.imu_rotation_real_ = []
        self.imu_acceleration_x_ = []
        self.imu_acceleration_y_ = []
        self.imu_acceleration_z_ = []
    
    def get_firmware_version(self):
        return self.firmware_version_
    
    def update_state(self, state: DataStreamState, new_value) -> DataStreamState:
        match state:
            case DataStreamState.IMU_STATUS:
                self.reset_()
                self.imu_status_ = new_value
                next_state: DataStreamState = DataStreamState.IMU_FILE_STATUS
                return next_state
            
            case DataStreamState.IMU_FILE_STATUS:
                self.imu_file_status_ = new_value
                next_state: DataStreamState = DataStreamState.IMU_COUNT
                return next_state
            
            case DataStreamState.IMU_COUNT:
                self.imu_count_ = new_value
                next_state: DataStreamState = DataStreamState.IMU_SLEEP_STATUS
                return next_state
            
            case DataStreamState.IMU_SLEEP_STATUS:
                self.imu_sleep_status_ = new_value
                next_state: DataStreamState = DataStreamState.FIRMWARE_VERSION
                return next_state
            
            case DataStreamState.FIRMWARE_VERSION:
                self.firmware_version_ = new_value
                next_state: DataStreamState = DataStreamState.SD_STATUS
                return next_state
            
            case DataStreamState.SD_STATUS:
                self.sd_status_ = new_value
                next_state: DataStreamState = DataStreamState.RTC_STATUS
                return next_state
            
            case DataStreamState.RTC_STATUS:
                self.rtc_status_ = new_value
                next_state: DataStreamState = DataStreamState.SECOND
                return next_state
            
            case DataStreamState.SECOND:
                self.second_ = new_value
                next_state: DataStreamState = DataStreamState.MINUTE
                return next_state
            
            case DataStreamState.MINUTE:
                self.minute_ = new_value
                next_state: DataStreamState = DataStreamState.HOUR
                return next_state
            
            case DataStreamState.HOUR:
                self.hour_ = new_value
                next_state: DataStreamState = DataStreamState.DAY
                return next_state
            
            case DataStreamState.DAY:
                self.day_ = new_value
                next_state: DataStreamState = DataStreamState.DAYOFWEEK
                return next_state
            
            case DataStreamState.DAYOFWEEK:
                self.day_of_week_ = new_value
                next_state: DataStreamState = DataStreamState.MONTH
                return next_state

            case DataStreamState.MONTH:
                self.month_ = new_value
                next_state: DataStreamState = DataStreamState.YEAR
                return next_state

            case DataStreamState.YEAR:
                self.year_ = new_value
                next_state: DataStreamState = DataStreamState.BATTERY_VOLTAGE_STATUS
                return next_state

            case DataStreamState.BATTERY_VOLTAGE_STATUS:
                self.battery_voltage_status_ = new_value
                next_state: DataStreamState = DataStreamState.BATTERY_VOLTAGE
                return next_state

            case DataStreamState.BATTERY_VOLTAGE:
                self.battery_voltage_ = new_value
                next_state: DataStreamState = DataStreamState.TEMP_SENSOR_COUNT
                return next_state

            case DataStreamState.TEMP_SENSOR_COUNT:
                self.temp_sensors_count_ = new_value
                next_state: DataStreamState = DataStreamState.TEMP_SENSOR_STATUS if self.temp_sensors_count_ > 0 else DataStreamState.HUMIDITY_SENSOR_COUNT
                return next_state

            case DataStreamState.TEMP_SENSOR_STATUS:
                self.temp_sensors_status_.append(new_value)
                next_state: DataStreamState = DataStreamState.TEMP_SENSOR_DATA
                return next_state

            case DataStreamState.TEMP_SENSOR_DATA:
                self.temp_sensors_data_.append(new_value)
                next_state: DataStreamState = DataStreamState.TEMP_SENSOR_STATUS if len(self.temp_sensors_data_) < self.temp_sensors_count_ else DataStreamState.HUMIDITY_SENSOR_COUNT
                return next_state

            case DataStreamState.HUMIDITY_SENSOR_COUNT:
                self.humidity_sensors_count_ = new_value
                next_state: DataStreamState = DataStreamState.HUMIDITY_SENSOR_STATUS if self.humidity_sensors_count_ > 0 else DataStreamState.PRESSURE_SENSOR_COUNT
                return next_state

            case DataStreamState.HUMIDITY_SENSOR_STATUS:
                self.humidity_sensors_status_.append(new_value)
                next_state: DataStreamState = DataStreamState.HUMIDITY_SENSOR_DATA
                return next_state

            case DataStreamState.HUMIDITY_SENSOR_DATA:
                self.humidity_sensors_data_.append(new_value)
                next_state: DataStreamState = DataStreamState.HUMIDITY_SENSOR_STATUS if len(self.humidity_sensors_data_) < self.humidity_sensors_count_ else DataStreamState.PRESSURE_SENSOR_COUNT
                return next_state

            case DataStreamState.PRESSURE_SENSOR_COUNT:
                self.pressure_sensors_count_ = new_value
                next_state: DataStreamState = DataStreamState.PRESSURE_SENSOR_STATUS if self.pressure_sensors_count_ > 0 else DataStreamState.UV_SENSOR_COUNT
                return next_state

            case DataStreamState.PRESSURE_SENSOR_STATUS:
                self.pressure_sensors_status_.append(new_value)
                next_state: DataStreamState = DataStreamState.PRESSURE_SENSOR_DATA
                return next_state

            case DataStreamState.PRESSURE_SENSOR_DATA:
                self.pressure_sensors_data_.append(new_value)
                next_state: DataStreamState = DataStreamState.PRESSURE_SENSOR_STATUS if len(self.pressure_sensors_data_) < self.pressure_sensors_count_ else DataStreamState.UV_SENSOR_COUNT
                return next_state

            case DataStreamState.UV_SENSOR_COUNT:
                self.uv_sensors_count_ = new_value
                next_state: DataStreamState = DataStreamState.UV_SENSOR_STATUS if self.uv_sensors_count_ > 0 else DataStreamState.VISIBLE_LIGHT_SENSOR_COUNT
                return next_state

            case DataStreamState.UV_SENSOR_STATUS:
                self.uv_sensors_status_.append(new_value)
                next_state: DataStreamState = DataStreamState.UV_SENSOR_DATA
                return next_state

            case DataStreamState.UV_SENSOR_DATA:
                self.uv_sensors_data_.append(new_value)
                next_state: DataStreamState = DataStreamState.UV_SENSOR_STATUS if len(self.uv_sensors_data_) < self.uv_sensors_count_ else DataStreamState.VISIBLE_LIGHT_SENSOR_COUNT
                return next_state

            case DataStreamState.VISIBLE_LIGHT_SENSOR_COUNT:
                self.visible_light_sensors_count_ = new_value
                next_state: DataStreamState = DataStreamState.VISIBLE_LIGHT_SENSOR_STATUS if self.visible_light_sensors_count_ > 0 else DataStreamState.IR_LIGHT_SENSOR_COUNT
                return next_state

            case DataStreamState.VISIBLE_LIGHT_SENSOR_STATUS:
                self.visible_light_sensors_status_.append(new_value)
                next_state: DataStreamState = DataStreamState.VISIBLE_LIGHT_SENSOR_DATA
                return next_state

            case DataStreamState.VISIBLE_LIGHT_SENSOR_DATA:
                self.visible_light_sensors_data_.append(new_value)
                next_state: DataStreamState = DataStreamState.VISIBLE_LIGHT_SENSOR_STATUS if len(self.visible_light_sensors_data_) < self.visible_light_sensors_count_ else DataStreamState.IR_LIGHT_SENSOR_COUNT
                return next_state

            case DataStreamState.IR_LIGHT_SENSOR_COUNT:
                self.ir_light_sensors_count_ = new_value
                next_state: DataStreamState = DataStreamState.IR_LIGHT_SENSOR_STATUS
                if self.ir_light_sensors_count_ == 0:
                    self.process_data_()
                    next_state = DataStreamState.IMU_ROTATION_I if self.imu_count_ > 0 else DataStreamState.FINISHED
                return next_state

            case DataStreamState.IR_LIGHT_SENSOR_STATUS:
                self.ir_light_sensors_status_.append(new_value)
                next_state: DataStreamState = DataStreamState.IR_LIGHT_SENSOR_DATA
                return next_state

            case DataStreamState.IR_LIGHT_SENSOR_DATA:
                self.ir_light_sensors_data_.append(new_value)
                next_state: DataStreamState = DataStreamState.IR_LIGHT_SENSOR_STATUS
                if len(self.ir_light_sensors_data_) == self.ir_light_sensors_count_:
                    self.process_data_()
                    next_state = DataStreamState.IMU_ROTATION_I if self.imu_count_ > 0 else DataStreamState.FINISHED
                return next_state

            case DataStreamState.IMU_ROTATION_I:
                self.imu_rotation_i_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ROTATION_J
                return next_state

            case DataStreamState.IMU_ROTATION_J:
                self.imu_rotation_j_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ROTATION_K
                return next_state

            case DataStreamState.IMU_ROTATION_K:
                self.imu_rotation_k_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ROTATION_REAL
                return next_state

            case DataStreamState.IMU_ROTATION_REAL:
                self.imu_rotation_real_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ACCELERATION_X
                return next_state

            case DataStreamState.IMU_ACCELERATION_X:
                self.imu_acceleration_x_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ACCELERATION_Y
                return next_state

            case DataStreamState.IMU_ACCELERATION_Y:
                self.imu_acceleration_y_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ACCELERATION_Z
                return next_state

            case DataStreamState.IMU_ACCELERATION_Z:
                self.imu_acceleration_z_.append(new_value)
                next_state: DataStreamState = DataStreamState.IMU_ROTATION_I
                if len(self.imu_acceleration_z_) == self.imu_count_:
                    self.process_imu_data_()
                    next_state = DataStreamState.FINISHED
                return next_state
    
    def process_data_(self) -> None:
        data_dict: dict = self.construct_data_dict_()
        threading.Thread(target=self.process_func_, args=(data_dict,)).start()
    
    def process_imu_data_(self) -> None:
        data_dict: dict = self.construct_imu_data_dict_()
        threading.Thread(target=self.process_imu_func_, args=(data_dict,)).start()
    
    @staticmethod
    def process_func_(data_dict: dict) -> None:
        logger.debug(f"Processed data: {data_dict}")
        send_data_to_backend(int(data_dict['firmware_version'] * 100), data_dict['temp_sensors_data'][1], data_dict['temp_sensors_data'][0], data_dict['uv_sensors_data'][0], data_dict['humidity_sensors_data'][0], data_dict['pressure_sensors_data'][0], data_dict['temp_sensors_data'][2], data_dict['temp_sensors_data'][3], data_dict['visible_light_sensors_data'][0], 0.0, 0.0, data_dict['battery_voltage'])
    
    @staticmethod
    def process_imu_func_(data_dict: dict) -> None:
        logger.debug(f"Processed imu data: {data_dict}")
        with open(IMU_LOG_FILE, 'a') as f:
            f.write(json.dumps(data_dict) + "\r\n")
        
    def construct_data_dict_(self) -> dict:
        return {
            'imu_status': self.imu_status_,
            'imu_file_status': self.imu_file_status_,
            'imu_count': self.imu_count_,
            'imu_sleep_status': self.imu_sleep_status_,
            'firmware_version': self.firmware_version_,
            'sd_status': self.sd_status_,
            'rtc_status': self.rtc_status_,
            'second': self.second_,
            'minute': self.minute_,
            'hour': self.hour_,
            'day': self.day_,
            'day_of_week': self.day_of_week_,
            'month': self.month_,
            'year': self.year_,
            'battery_voltage_status': self.battery_voltage_status_,
            'battery_voltage': self.battery_voltage_,
            'temp_sensors_count': self.temp_sensors_count_,
            'temp_sensors_status': self.temp_sensors_status_,
            'temp_sensors_data': self.temp_sensors_data_,
            'humidity_sensors_count': self.humidity_sensors_count_,
            'humidity_sensors_status': self.humidity_sensors_status_,
            'humidity_sensors_data': self.humidity_sensors_data_,
            'pressure_sensors_count': self.pressure_sensors_count_,
            'pressure_sensors_status': self.pressure_sensors_status_,
            'pressure_sensors_data': self.pressure_sensors_data_,
            'uv_sensors_count': self.uv_sensors_count_,
            'uv_sensors_status': self.uv_sensors_status_,
            'uv_sensors_data': self.uv_sensors_data_,
            'visible_light_sensors_count': self.visible_light_sensors_count_,
            'visible_light_sensors_status': self.visible_light_sensors_status_,
            'visible_light_sensors_data': self.visible_light_sensors_data_,
            'ir_light_sensors_count': self.ir_light_sensors_count_,
            'ir_light_sensors_status': self.ir_light_sensors_status_,
            'ir_light_sensors_data': self.ir_light_sensors_data_
        }
    
    def construct_imu_data_dict_(self) -> dict:
        return {
            'imu_rotation_i': self.imu_rotation_i_,
            'imu_rotation_j': self.imu_rotation_j_,
            'imu_rotation_k': self.imu_rotation_k_,
            'imu_rotation_real': self.imu_rotation_real_,
            'imu_acceleration_x': self.imu_acceleration_x_,
            'imu_acceleration_y': self.imu_acceleration_y_,
            'imu_acceleration_z': self.imu_acceleration_z_
        }