/*
The MIT License (MIT)

Copyright (c) 2014 Drew Folta

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <TextDevices.h>
#include <PulseinDevice.h>
#include <ShiftersDevice.h>
#include <ShortcutsDevice.h>
#include <TimersDevice.h>
#include <WatchersDevice.h>

TextDevices::Devices            devices;
TextDevices::PulseinDevice      pulseinDevice;
TextDevices::ShiftersDevice     shiftersDevice;
TextDevices::ShortcutsDevice    shortcutsDevice;
TextDevices::TimersDevice       timersDevice;
TextDevices::WatchersDevice     watchersDevice;

void setup() {
    Serial.begin(9600);
    devices.setup(&Serial);
    devices.registerDevice(&pulseinDevice);
    devices.registerDevice(&shiftersDevice);
    devices.registerDevice(&shortcutsDevice);
    devices.registerDevice(&timersDevice);
    devices.registerDevice(&watchersDevice);
}

void loop() {
    devices.loop();
}

