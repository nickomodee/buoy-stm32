#include "PAL.h"
// This demo explores two reports (SH2_ARVR_STABILIZED_RV and SH2_GYRO_INTEGRATED_RV) both can be used to give 
// quartenion and euler (yaw, pitch roll) angles.  Toggle the FAST_MODE define to see other report.  
// Note sensor_value.status gives calibration accuracy (which improves over time)
#include "BNO085.h"

#define sq(x) ((x)*(x))
#define RAD_TO_DEG 57.295779513082320876798154814105

#define BNO085_RESET -1

struct euler_t {
    float yaw;
    float pitch;
    float roll;
} ypr;

BNO085 bno085(BNO085_RESET);

#ifdef FAST_MODE
    // Top frequency is reported to be 1000Hz (but freq is somewhat variable)
    sh2_SensorId_t report_type = SH2_GYRO_INTEGRATED_RV;
    long report_interval_us = 2000;
#else
    // Top frequency is about 250Hz but this report is more accurate
    sh2_SensorId_t report_type = SH2_ARVR_STABILIZED_RV;
    long report_interval_us = 5000;
#endif

void set_reports(sh2_SensorId_t report_type, long report_interval) {
    PAL_SERIAL.println("Setting desired reports");
    if (!bno085.enable_report(report_type, report_interval)) {
        PAL_SERIAL.println("Could not enable stabilized remote vector");
    }
}

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

    set_reports(report_type, report_interval_us);

    PAL_SERIAL.println("Reading events");
    PAL_DELAY(100);
}

void quaternion_to_euler(float qr, float qi, float qj, float qk, euler_t* ypr, bool degrees = false) {
    float sqr = sq(qr);
    float sqi = sq(qi);
    float sqj = sq(qj);
    float sqk = sq(qk);

    ypr->yaw = atan2(2.0 * (qi * qj + qk * qr), (sqi - sqj - sqk + sqr));
    ypr->pitch = asin(-2.0 * (qi * qk - qj * qr) / (sqi + sqj + sqk + sqr));
    ypr->roll = atan2(2.0 * (qj * qk + qi * qr), (-sqi - sqj + sqk + sqr));

    if (degrees) {
      ypr->yaw *= RAD_TO_DEG;
      ypr->pitch *= RAD_TO_DEG;
      ypr->roll *= RAD_TO_DEG;
    }
}

void quaternion_to_euler_RV(sh2_RotationVectorWAcc_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternion_to_euler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

void quaternion_to_euler_GI(sh2_GyroIntegratedRV_t* rotational_vector, euler_t* ypr, bool degrees = false) {
    quaternion_to_euler(rotational_vector->real, rotational_vector->i, rotational_vector->j, rotational_vector->k, ypr, degrees);
}

void loop() {
    if (bno085.was_reset()) {
        PAL_SERIAL.print("sensor was reset");
        set_reports(report_type, report_interval_us);
    }

    if (bno085.get_sensor_event()) {
        sh2_SensorValue_t& sensor_value = *bno085.get_sensor_value_ptr();

        // in this demo only one report type will be received depending on FAST_MODE define (above)
        switch (sensor_value.sensorId) {
            case SH2_ARVR_STABILIZED_RV:
                quaternion_to_euler_RV(&sensor_value.un.arvrStabilizedRV, &ypr, true);
            
            case SH2_GYRO_INTEGRATED_RV:
                // faster (more noise?)
                quaternion_to_euler_GI(&sensor_value.un.gyroIntegratedRV, &ypr, true);

            break;
        }
        
        static long last = 0;
        uint32_t now = PAL_MILLISECONDS();
        PAL_SERIAL.print(now - last);             PAL_SERIAL.print("\t");
        last = now;
        PAL_SERIAL.print(sensor_value.status);     PAL_SERIAL.print("\t");  // This is accuracy in the range of 0 to 3
        PAL_SERIAL.print(ypr.yaw);                PAL_SERIAL.print("\t");
        PAL_SERIAL.print(ypr.pitch);              PAL_SERIAL.print("\t");
        PAL_SERIAL.println(ypr.roll);
    }
}