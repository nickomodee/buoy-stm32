#include "PAL.h"
#include "SI1145.h"
#include "UVSensor.h"
#include "VisibleLightSensor.h"
#include "IRLightSensor.h"

SI1145 si1145;
UVSensor& uv_sensor = si1145;
VisibleLightSensor& visible_light_sensor = si1145;
IRLightSensor& ir_light_sensor = si1145;

void setup() {
    PAL_SERIAL.begin(9600);
    Wire.begin();
    while (!si1145.init()) {
        PAL_SERIAL.println("SI1145 initialisation failed");
        PAL_DELAY(1000);
    }
}

void loop() {
    const float uv_index = uv_sensor.read_uv_index();

    const uint16_t visible_counts = visible_light_sensor.read_visible();
    const uint16_t ir_counts = ir_light_sensor.read_ir();
    const float lux = si1145.calculate_lux(visible_counts, ir_counts);

    PAL_SERIAL.print("UV Index: ");
    PAL_SERIAL.print(uv_index / 100.0f, 2); // 2 d.p.
    PAL_SERIAL.println();

    PAL_SERIAL.print("Visible Light (counts): ");
    PAL_SERIAL.print(visible_counts);
    PAL_SERIAL.println();

    PAL_SERIAL.print("IR Light (counts): ");
    PAL_SERIAL.print(ir_counts);
    PAL_SERIAL.println();

    PAL_SERIAL.print("Light intensity (lux): ");
    PAL_SERIAL.print(lux);
    PAL_SERIAL.println();

    PAL_SERIAL.println();
    PAL_DELAY(2000);
}