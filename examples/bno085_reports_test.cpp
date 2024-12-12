// Basic demo for readings from Adafruit BNO08x
#include "PAL.h"
#include "BNO085.h"

#define BNO085_RESET -1

BNO085 bno085(BNO085_RESET);

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

    for (int n = 0; n < bno085.prod_ids.numEntries; n++) {
        PAL_SERIAL.print("Part ");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swPartNumber);
        PAL_SERIAL.print(": Version :");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swVersionMajor);
        PAL_SERIAL.print(".");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swVersionMinor);
        PAL_SERIAL.print(".");
        PAL_SERIAL.print(bno085.prod_ids.entry[n].swVersionPatch);
        PAL_SERIAL.print(" Build ");
        PAL_SERIAL.println(bno085.prod_ids.entry[n].swBuildNumber);
    }

    set_reports();

    PAL_SERIAL.println("Reading events");
    PAL_DELAY(100);
}

// here is where you define the sensor outputs you want to receive
void set_reports() {
    PAL_SERIAL.println("Setting desired reports");
    if (!bno085.enable_report(SH2_ACCELEROMETER)) {
        PAL_SERIAL.println("Could not enable accelerometer");
    }
    if (!bno085.enable_report(SH2_GYROSCOPE_CALIBRATED)) {
        PAL_SERIAL.println("Could not enable gyroscope");
    }
    if (!bno085.enable_report(SH2_MAGNETIC_FIELD_CALIBRATED)) {
        PAL_SERIAL.println("Could not enable magnetic field calibrated");
    }
    if (!bno085.enable_report(SH2_LINEAR_ACCELERATION)) {
        PAL_SERIAL.println("Could not enable linear acceleration");
    }
    if (!bno085.enable_report(SH2_GRAVITY)) {
        PAL_SERIAL.println("Could not enable gravity vector");
    }
    if (!bno085.enable_report(SH2_ROTATION_VECTOR)) {
        PAL_SERIAL.println("Could not enable rotation vector");
    }
    if (!bno085.enable_report(SH2_GEOMAGNETIC_ROTATION_VECTOR)) {
        PAL_SERIAL.println("Could not enable geomagnetic rotation vector");
    }
    if (!bno085.enable_report(SH2_GAME_ROTATION_VECTOR)) {
        PAL_SERIAL.println("Could not enable game rotation vector");
    }
    if (!bno085.enable_report(SH2_STEP_COUNTER)) {
        PAL_SERIAL.println("Could not enable step counter");
    }
    if (!bno085.enable_report(SH2_STABILITY_CLASSIFIER)) {
        PAL_SERIAL.println("Could not enable stability classifier");
    }
    if (!bno085.enable_report(SH2_RAW_ACCELEROMETER)) {
        PAL_SERIAL.println("Could not enable raw accelerometer");
    }
    if (!bno085.enable_report(SH2_RAW_GYROSCOPE)) {
        PAL_SERIAL.println("Could not enable raw gyroscope");
    }
    if (!bno085.enable_report(SH2_RAW_MAGNETOMETER)) {
        PAL_SERIAL.println("Could not enable raw magnetometer");
    }
    if (!bno085.enable_report(SH2_SHAKE_DETECTOR)) {
        PAL_SERIAL.println("Could not enable shake detector");
    }
    if (!bno085.enable_report(SH2_PERSONAL_ACTIVITY_CLASSIFIER)) {
        PAL_SERIAL.println("Could not enable personal activity classifier");
    }
}

void print_activity(uint8_t activity_id) {
    switch (activity_id) {
        case PAC_UNKNOWN:
            PAL_SERIAL.print("Unknown");
            break;

        case PAC_IN_VEHICLE:
            PAL_SERIAL.print("In Vehicle");
            break;

        case PAC_ON_BICYCLE:
            PAL_SERIAL.print("On Bicycle");
            break;

        case PAC_ON_FOOT:
            PAL_SERIAL.print("On Foot");
            break;

        case PAC_STILL:
            PAL_SERIAL.print("Still");
            break;

        case PAC_TILTING:
            PAL_SERIAL.print("Tilting");
            break;

        case PAC_WALKING:
            PAL_SERIAL.print("Walking");
            break;

        case PAC_RUNNING:
            PAL_SERIAL.print("Running");
            break;

        case PAC_ON_STAIRS:
            PAL_SERIAL.print("On Stairs");
            break;

        default:
            PAL_SERIAL.print("NOT LISTED");
    }

    PAL_SERIAL.print(" (");
    PAL_SERIAL.print(activity_id);
    PAL_SERIAL.print(")");
}

void loop() {
    PAL_DELAY(10);

    if (bno085.was_reset()) {
        PAL_SERIAL.print("sensor was reset ");
        set_reports();
    }

    if (!bno085.get_sensor_event()) {
        return;
    }

    sh2_SensorValue_t& sensor_value = *bno085.get_sensor_value_ptr();

    switch (sensor_value.sensorId) {
        case SH2_ACCELEROMETER:
            PAL_SERIAL.print("Accelerometer - x: ");
            PAL_SERIAL.print(sensor_value.un.accelerometer.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.accelerometer.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.accelerometer.z);
            break;

        case SH2_GYROSCOPE_CALIBRATED:
            PAL_SERIAL.print("Gyro - x: ");
            PAL_SERIAL.print(sensor_value.un.gyroscope.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.gyroscope.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.gyroscope.z);
            break;

        case SH2_MAGNETIC_FIELD_CALIBRATED:
            PAL_SERIAL.print("Magnetic Field - x: ");
            PAL_SERIAL.print(sensor_value.un.magneticField.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.magneticField.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.magneticField.z);
            break;

        case SH2_LINEAR_ACCELERATION:
            PAL_SERIAL.print("Linear Acceration - x: ");
            PAL_SERIAL.print(sensor_value.un.linearAcceleration.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.linearAcceleration.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.linearAcceleration.z);
            break;

        case SH2_GRAVITY:
            PAL_SERIAL.print("Gravity - x: ");
            PAL_SERIAL.print(sensor_value.un.gravity.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.gravity.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.gravity.z);
            break;

        case SH2_ROTATION_VECTOR:
            PAL_SERIAL.print("Rotation Vector - r: ");
            PAL_SERIAL.print(sensor_value.un.rotationVector.real);
            PAL_SERIAL.print(" i: ");
            PAL_SERIAL.print(sensor_value.un.rotationVector.i);
            PAL_SERIAL.print(" j: ");
            PAL_SERIAL.print(sensor_value.un.rotationVector.j);
            PAL_SERIAL.print(" k: ");
            PAL_SERIAL.println(sensor_value.un.rotationVector.k);
            break;

        case SH2_GEOMAGNETIC_ROTATION_VECTOR:
            PAL_SERIAL.print("Geo-Magnetic Rotation Vector - r: ");
            PAL_SERIAL.print(sensor_value.un.geoMagRotationVector.real);
            PAL_SERIAL.print(" i: ");
            PAL_SERIAL.print(sensor_value.un.geoMagRotationVector.i);
            PAL_SERIAL.print(" j: ");
            PAL_SERIAL.print(sensor_value.un.geoMagRotationVector.j);
            PAL_SERIAL.print(" k: ");
            PAL_SERIAL.println(sensor_value.un.geoMagRotationVector.k);
            break;

        case SH2_GAME_ROTATION_VECTOR:
            PAL_SERIAL.print("Game Rotation Vector - r: ");
            PAL_SERIAL.print(sensor_value.un.gameRotationVector.real);
            PAL_SERIAL.print(" i: ");
            PAL_SERIAL.print(sensor_value.un.gameRotationVector.i);
            PAL_SERIAL.print(" j: ");
            PAL_SERIAL.print(sensor_value.un.gameRotationVector.j);
            PAL_SERIAL.print(" k: ");
            PAL_SERIAL.println(sensor_value.un.gameRotationVector.k);
            break;

        case SH2_STEP_COUNTER:
            PAL_SERIAL.print("Step Counter - steps: ");
            PAL_SERIAL.print(sensor_value.un.stepCounter.steps);
            PAL_SERIAL.print(" latency: ");
            PAL_SERIAL.println(sensor_value.un.stepCounter.latency);
            break;

        case SH2_STABILITY_CLASSIFIER: {
            PAL_SERIAL.print("Stability Classification: ");
            sh2_StabilityClassifier_t stability = sensor_value.un.stabilityClassifier;
            switch (stability.classification) {
                case STABILITY_CLASSIFIER_UNKNOWN:
                    PAL_SERIAL.println("Unknown");
                    break;

                case STABILITY_CLASSIFIER_ON_TABLE:
                    PAL_SERIAL.println("On Table");
                    break;

                case STABILITY_CLASSIFIER_STATIONARY:
                    PAL_SERIAL.println("Stationary");
                    break;

                case STABILITY_CLASSIFIER_STABLE:
                    PAL_SERIAL.println("Stable");
                    break;

                case STABILITY_CLASSIFIER_MOTION:
                    PAL_SERIAL.println("In Motion");
                    break;
            }
            break;
        }

        case SH2_RAW_ACCELEROMETER:
            PAL_SERIAL.print("Raw Accelerometer - x: ");
            PAL_SERIAL.print(sensor_value.un.rawAccelerometer.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.rawAccelerometer.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.rawAccelerometer.z);
            break;

        case SH2_RAW_GYROSCOPE:
            PAL_SERIAL.print("Raw Gyro - x: ");
            PAL_SERIAL.print(sensor_value.un.rawGyroscope.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.rawGyroscope.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.rawGyroscope.z);
            break;

        case SH2_RAW_MAGNETOMETER:
            PAL_SERIAL.print("Raw Magnetic Field - x: ");
            PAL_SERIAL.print(sensor_value.un.rawMagnetometer.x);
            PAL_SERIAL.print(" y: ");
            PAL_SERIAL.print(sensor_value.un.rawMagnetometer.y);
            PAL_SERIAL.print(" z: ");
            PAL_SERIAL.println(sensor_value.un.rawMagnetometer.z);
            break;

        case SH2_SHAKE_DETECTOR: {
            PAL_SERIAL.print("Shake Detector - shake detected on axis: ");
            sh2_ShakeDetector_t detection = sensor_value.un.shakeDetector;
            switch (detection.shake) {
                case SHAKE_X:
                    PAL_SERIAL.println("X");
                    break;

                case SHAKE_Y:
                    PAL_SERIAL.println("Y");
                    break;

                case SHAKE_Z:
                    PAL_SERIAL.println("Z");
                    break;

                default:
                    PAL_SERIAL.println("None");
                    break;
            }
        }

        case SH2_PERSONAL_ACTIVITY_CLASSIFIER: {
            sh2_PersonalActivityClassifier_t activity = sensor_value.un.personalActivityClassifier;
            PAL_SERIAL.print("Activity classification - Most likely: ");
            print_activity(activity.mostLikelyState);
            PAL_SERIAL.println("");

            PAL_SERIAL.println("Confidences:");
            // if PAC_OPTION_COUNT is ever > 10, we'll need to
            // care about page
            for (uint8_t i = 0; i < PAC_OPTION_COUNT; i++) {
                PAL_SERIAL.print("\t");
                print_activity(i);
                PAL_SERIAL.print(": ");
                PAL_SERIAL.println(activity.confidence[i]);
            }
        }
    }
}