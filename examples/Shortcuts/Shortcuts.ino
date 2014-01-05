#include <TextDevices.h>
#include <ShortcutsDevice.h>

TextDevices::Devices devices;
TextDevices::ShortcutsDevice shortcuts;

void setup() {
    Serial.begin(9600);
    devices.setup(&Serial);
    devices.registerDevice(&shortcuts);
}

void loop() {
    devices.loop();
}

