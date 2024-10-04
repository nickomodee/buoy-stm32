#include "PAL.h"
#include "SI1145.h"
#include "UVSensor.h"
#include "VisibleLuxSensor.h"
#include "IRLuxSensor.h"

SI1145 si1145;
UVSensor& uv_sensor = si1145;
VisibleLuxSensor& visible_lux_sensor = si1145;
IRLuxSensor& ir_lux_sensor = si1145;

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();
    while (!si1145.init()) {
        PAL_DELAY(1000);
    }
}

void loop() {
    double uv_index = uv_sensor.read_uv_index();
    double visible_lux = visible_lux_sensor.read_visible_lux();
    double ir_lux = ir_lux_sensor.read_ir_lux();

    PAL_SERIAL.print("UV Index: ");
    PAL_SERIAL.print(uv_index, 2); // 2 d.p.
    PAL_SERIAL.println();

    PAL_SERIAL.print("Visible Light (Lux): ");
    PAL_SERIAL.print(visible_lux, 2); // 2 d.p.
    PAL_SERIAL.println();

    PAL_SERIAL.print("IR Light (Lux): ");
    PAL_SERIAL.print(ir_lux, 2); // 2 d.p.
    PAL_SERIAL.println();

    PAL_SERIAL.println();
    PAL_DELAY(2000);
}