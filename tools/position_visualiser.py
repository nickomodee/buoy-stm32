# ported from: https://raw.githubusercontent.com/xioTechnologies/Oscillatory-Motion-Tracking-With-x-IMU/refs/heads/master/Script.m
import numpy as np
import matplotlib.pyplot as plt
from scipy.integrate import simpson
from scipy.signal import butter, filtfilt

import json
import sys
import os
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from server_code.ServerState import IMU_LOG_FILE

def get_imu_data_dict():
    with open(IMU_LOG_FILE, "r") as f:
        last_line = ""
        for line in f:
            if line.strip():
                last_line = line.strip()
    imu_data_dict = json.loads(last_line)
    return imu_data_dict

imu_data_dict = get_imu_data_dict()
imu_acceleration_x = imu_data_dict['imu_acceleration_x']
imu_acceleration_y = imu_data_dict['imu_acceleration_y']
imu_acceleration_z = imu_data_dict['imu_acceleration_z']

# high pass filter for drift (since it is cyclic motion we know it will return to 0)
def high_pass_filter(data, cutoff, fs, order=1):
    nyquist = 0.5 * fs
    normal_cutoff = cutoff / nyquist
    b, a = butter(order, normal_cutoff, btype='high', analog=False)
    return filtfilt(b, a, data)

# better than trapezium method, approximate using parabolas between points instead of a straight line. source: chatgpt
def simpsons_rule_integration(data, time_delta):
    n = len(data)
    integral = np.zeros_like(data)
    for i in range(1, n - 1, 2):
        integral[i] = integral[i - 1] + (time_delta / 3) * (data[i - 1] + 4 * data[i] + data[i+1])
    integral[-1] = integral[-2] + (time_delta / 2) * (data[-2] + data[-1]) # handle the last poiint
    return integral

sample_period = 0.05
sample_frequency = 1 / sample_period

# integrate the linear accelerations to get linear velocities
velocity_x = simpsons_rule_integration(imu_acceleration_x, sample_period)
velocity_y = simpsons_rule_integration(imu_acceleration_y, sample_period)
velocity_z = simpsons_rule_integration(imu_acceleration_z, sample_period)

# filter using the settings from the matlab code
cutoff_velocity = 0.001 # 0.1 Hz. we should test what works best
velocity_x_filtered = high_pass_filter(velocity_x, cutoff_velocity, sample_frequency)
velocity_y_filtered = high_pass_filter(velocity_y, cutoff_velocity, sample_frequency)
velocity_z_filtered = high_pass_filter(velocity_z, cutoff_velocity, sample_frequency)

# plot filtered velocities
plt.figure(figsize=(10, 6))
plt.plot(velocity_x_filtered, 'r', label='vx')
plt.plot(velocity_y_filtered, 'g', label='vy')
plt.plot(velocity_z_filtered, 'b', label='vz')
plt.xlabel('Sample')
plt.ylabel('Velocity (ms^-1)')
plt.title('High pass filtered linear velocity')
plt.legend()
plt.grid(True)
plt.show()

# integrate the filtered linear velocities to get an estimate for displacement
displacement_x = simpsons_rule_integration(velocity_x_filtered, sample_period)
displacement_y = simpsons_rule_integration(velocity_y_filtered, sample_period)
displacement_z = simpsons_rule_integration(velocity_z_filtered, sample_period)

# filter the estimate to remove drift
cutoff_position = 0.001 # 0.1 Hz. we should test what works best
displacement_x_filtered = high_pass_filter(displacement_x, cutoff_position, sample_frequency)
displacement_y_filtered = high_pass_filter(displacement_y, cutoff_position, sample_frequency)
displacement_z_filtered = high_pass_filter(displacement_z, cutoff_position, sample_frequency)

# filtered displacement
plt.figure(figsize=(10, 6))
plt.plot(displacement_x_filtered, 'r', label='sx')
plt.plot(displacement_y_filtered, 'g', label='sy')
plt.plot(displacement_z_filtered, 'b', label='sz')
plt.xlabel('Sample')
plt.ylabel('Displacement (m)')
plt.title('High pass filtered linear displacement')
plt.legend()
plt.grid(True)
plt.show()

time = np.arange(0, len(imu_acceleration_x) * sample_period, sample_period)

plt.figure(figsize=(12, 12))

# raw linear acceleration
plt.subplot(3, 3, 1)
plt.plot(time, imu_acceleration_x, 'r')
plt.title('ax')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (ms^-2)')
plt.grid(True)

plt.subplot(3, 3, 2)
plt.plot(time, imu_acceleration_y, 'g')
plt.title('ay')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (ms^-2)')
plt.grid(True)

plt.subplot(3, 3, 3)
plt.plot(time, imu_acceleration_z, 'b')
plt.title('az')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (ms^-2)')
plt.grid(True)

# filtered velocity
plt.subplot(3, 3, 4)
plt.plot(time, velocity_x_filtered, 'r')
plt.title('vx (filtered)')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (ms^-1)')
plt.grid(True)

plt.subplot(3, 3, 5)
plt.plot(time, velocity_y_filtered, 'g')
plt.title('vy (filtered)')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (ms^-1)')
plt.grid(True)

plt.subplot(3, 3, 6)
plt.plot(time, velocity_z_filtered, 'b')
plt.title('vz (filtered)')
plt.xlabel('Time (s)')
plt.ylabel('Velocity (ms^-1)')
plt.grid(True)

# filtered displacement
plt.subplot(3, 3, 7)
plt.plot(time, displacement_x_filtered, 'r')
plt.title('sx (filtered)')
plt.xlabel('Time (s)')
plt.ylabel('Displacement (m)')
plt.grid(True)

plt.subplot(3, 3, 8)
plt.plot(time, displacement_y_filtered, 'g')
plt.title('sy (filtered)')
plt.xlabel('Time (s)')
plt.ylabel('Displacement (m)')
plt.grid(True)

plt.subplot(3, 3, 9)
plt.plot(time, displacement_z_filtered, 'b')
plt.title('sz (filtered)')
plt.xlabel('Time (s)')
plt.ylabel('Displacement (m)')
plt.grid(True)

plt.tight_layout()
plt.show()