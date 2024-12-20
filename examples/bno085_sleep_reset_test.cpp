#include "PAL.h"
#include "BNO085.h"
#include <math.h>

#define BNO085_RESET 11

BNO085 bno085(BNO085_RESET);

// Global variables for sensor values
float gyro_x = 0, gyro_y = 0, gyro_z = 0;
float mag_x = 0, mag_y = 0, mag_z = 0;
float lin_acc_x = 0, lin_acc_y = 0, lin_acc_z = 0;
float total_lin_acc_x = 0, total_lin_acc_y = 0, total_lin_acc_z = 0;
float gravity_x = 0, gravity_y = 0, gravity_z = 0;
float rot_r = 0, rot_i = 0, rot_j = 0, rot_k = 0;
float roll = 0, pitch = 0, yaw = 0;

uint32_t last_print_time = 0;
constexpr uint32_t print_interval = 1000;

constexpr uint32_t RESET_OFFSET = 10000;
constexpr uint32_t SLEEP_OFFSET = 5000;
constexpr uint32_t WAKE_OFFSET  = 7500;
constexpr uint32_t ACTION_PERIOD = 10000;

static uint32_t next_time_reset = PAL_MILLISECONDS() + RESET_OFFSET;
static uint32_t next_time_sleep = PAL_MILLISECONDS() + SLEEP_OFFSET;
static uint32_t next_time_wake  = PAL_MILLISECONDS() + WAKE_OFFSET;

void set_reports();

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();

    PAL_SERIAL.println("BNO085 test");

    if (!bno085.begin()) {
        PAL_SERIAL.println("Failed to find BNO085 chip");
        while (1) {
            PAL_DELAY(10);
        }
    }
    PAL_SERIAL.println("BNO085 found");

    set_reports();

    PAL_SERIAL.println("Reading events");
    PAL_DELAY(100);
}

// set the desired sensor reports
void set_reports() {
    bno085.enable_report(SH2_GYROSCOPE_CALIBRATED, 10000);
    bno085.enable_report(SH2_MAGNETIC_FIELD_CALIBRATED, 10000);
    bno085.enable_report(SH2_LINEAR_ACCELERATION, 10000);
    bno085.enable_report(SH2_GRAVITY, 10000);
    bno085.enable_report(SH2_ROTATION_VECTOR, 10000);
}

void loop() {
    const uint32_t curr_time = PAL_MILLISECONDS();

    if (curr_time >= next_time_reset) {
        // we don't actually do both reset actions in the real program but this is just to test if they work
        PAL_SERIAL.print("Software reset success: ");
        PAL_SERIAL.println(bno085.soft_reset() ? "true" : "false");
        bno085.hardware_reset();
        PAL_SERIAL.println("Hardware reset");
        
        next_time_reset += ACTION_PERIOD;
    }

    if (curr_time >= next_time_sleep) {
        PAL_SERIAL.print("Sleep success: ");
        PAL_SERIAL.println(bno085.sleep() ? "true" : "false");
        
        next_time_sleep += ACTION_PERIOD;
    }
    
    if (curr_time >= next_time_wake) {
        PAL_SERIAL.print("Wake success: ");
        PAL_SERIAL.println(bno085.wake_up() ? "true" : "false"); // we can also wake by just resettin
        
        next_time_wake += ACTION_PERIOD;
    }

    if (bno085.was_reset()) {
        PAL_SERIAL.println("Sensor was reset");
        set_reports();
    }

    if (!bno085.get_sensor_event()) {
        return;
    }

    sh2_SensorValue_t& sensor_value = *bno085.get_sensor_value_ptr();

    switch (sensor_value.sensorId) {
        case SH2_GYROSCOPE_CALIBRATED:
            gyro_x = sensor_value.un.gyroscope.x;
            gyro_y = sensor_value.un.gyroscope.y;
            gyro_z = sensor_value.un.gyroscope.z;
            break;

        case SH2_MAGNETIC_FIELD_CALIBRATED:
            mag_x = sensor_value.un.magneticField.x;
            mag_y = sensor_value.un.magneticField.y;
            mag_z = sensor_value.un.magneticField.z;
            break;

        case SH2_LINEAR_ACCELERATION:
            lin_acc_x = sensor_value.un.linearAcceleration.x;
            lin_acc_y = sensor_value.un.linearAcceleration.y;
            lin_acc_z = sensor_value.un.linearAcceleration.z;
            total_lin_acc_x += lin_acc_x;
            total_lin_acc_y += lin_acc_y;
            total_lin_acc_z += lin_acc_z;
            break;

        case SH2_GRAVITY:
            gravity_x = sensor_value.un.gravity.x;
            gravity_y = sensor_value.un.gravity.y;
            gravity_z = sensor_value.un.gravity.z;
            break;

        case SH2_ROTATION_VECTOR:
            rot_r = sensor_value.un.rotationVector.real;
            rot_i = sensor_value.un.rotationVector.i;
            rot_j = sensor_value.un.rotationVector.j;
            rot_k = sensor_value.un.rotationVector.k;

            // calculate orientation angles from quaternion
            float roll_numerator = 2 * (rot_r * rot_i + rot_j * rot_k);
            float roll_denominator = 1 - 2 * (rot_i * rot_i + rot_j * rot_j);
            roll = atan2(roll_numerator, roll_denominator) * (180.0 / M_PI);

            float pitch_value = 2 * (rot_r * rot_j - rot_k * rot_i);
            pitch_value = PAL_CONSTRAIN(pitch_value, -1.0, 1.0);
            pitch = asin(pitch_value) * (180.0 / M_PI);

            float yaw_numerator = 2 * (rot_r * rot_k + rot_i * rot_j);
            float yaw_denominator = 1 - 2 * (rot_j * rot_j + rot_k * rot_k);
            yaw = atan2(yaw_numerator, yaw_denominator) * (180.0 / M_PI);
            break;
    }

    // Print values every second
    unsigned long current_time = PAL_MILLISECONDS();
    if (current_time - last_print_time >= print_interval) {
        last_print_time = current_time;

        PAL_SERIAL.println("Sensor Values:");
        PAL_SERIAL.print("Gyroscope - x: "); PAL_SERIAL.print(gyro_x);
        PAL_SERIAL.print(" y: "); PAL_SERIAL.print(gyro_y);
        PAL_SERIAL.print(" z: "); PAL_SERIAL.println(gyro_z);

        PAL_SERIAL.print("Magnetic Field - x: "); PAL_SERIAL.print(mag_x);
        PAL_SERIAL.print(" y: "); PAL_SERIAL.print(mag_y);
        PAL_SERIAL.print(" z: "); PAL_SERIAL.println(mag_z);

        PAL_SERIAL.print("Linear Acceleration - x: "); PAL_SERIAL.print(lin_acc_x);
        PAL_SERIAL.print(" y: "); PAL_SERIAL.print(lin_acc_y);
        PAL_SERIAL.print(" z: "); PAL_SERIAL.println(lin_acc_z);
        
        PAL_SERIAL.print("Total linear Acceleration - x: "); PAL_SERIAL.print(total_lin_acc_x);
        PAL_SERIAL.print(" y: "); PAL_SERIAL.print(total_lin_acc_y);
        PAL_SERIAL.print(" z: "); PAL_SERIAL.println(total_lin_acc_z);

        PAL_SERIAL.print("Gravity - x: "); PAL_SERIAL.print(gravity_x);
        PAL_SERIAL.print(" y: "); PAL_SERIAL.print(gravity_y);
        PAL_SERIAL.print(" z: "); PAL_SERIAL.println(gravity_z);

        PAL_SERIAL.println("Orientation:");
        PAL_SERIAL.print("Roll (X-axis): "); PAL_SERIAL.print(roll); PAL_SERIAL.println(" degrees");
        PAL_SERIAL.print("Pitch (Y-axis): "); PAL_SERIAL.print(pitch); PAL_SERIAL.println(" degrees");
        PAL_SERIAL.print("Yaw (Z-axis / Heading): "); PAL_SERIAL.print(yaw); PAL_SERIAL.println(" degrees");
    }
}