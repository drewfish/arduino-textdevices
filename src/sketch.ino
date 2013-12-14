
#include <TextDevices.h>


TextDevices::Devices devices;


void setup() {
    Serial.begin(9600);
    devices.setup(&Serial);
}


void loop() {
    devices.loop();
}


