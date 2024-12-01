#include "PAL.h"

void setup() {
    PAL_SERIAL.begin(9600);

    PAL_SERIAL.println(PAL_RANDOMSEED_INIT_ENTROPY());
}

void loop() {
    PAL_SERIAL.println(PAL_RANDOM(UINT32_MAX));
    PAL_DELAY(1000);
}