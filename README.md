TextDevices
===========

With this library you can type text commands on the serial console to set pins high or low, read from pins, be told when a pin changes, etc.

This is a library to use in your own sketches.
It's possible to use just this library -- any of the [example sketches](https://github.com/drewfish/arduino-textdevices/tree/master/examples/) can be compiled without change and uploaded to an arduino to get the functionality of this library.

This library comes in components (called "devices") which can be used to load only the funtionality you need.
The [examples/All](https://github.com/drewfish/arduino-textdevices/tree/master/examples/All/) shows how to use all the devices in this library.
Other examples show how to use only certain devices.


### Installing
Follow the [standard instructions](http://arduino.cc/en/Guide/Libraries) on how to install libraries.
You can download this library from [https://github.com/drewfish/arduino-textdevices/archive/master.zip](https://github.com/drewfish/arduino-textdevices/archive/master.zip).
(If you are familiar with git you can `git clone` this repo right into you libraries directory.)

To add this library to a sketch choose the `Sketch` menu, then `Import Library...`, then `TextDevices`. That will add the following to your sketch:

```cpp
#include <PinsDevice.h>
#include <PulseinDevice.h>
#include <ShiftersDevice.h>
#include <ShortcutsDevice.h>
#include <TextDevices.h>
#include <TimersDevice.h>
#include <WatchersDevice.h>
```

For the basics you only need `#include <TextDevices.h>`.
See the various [examples](https://github.com/drewfish/arduino-textdevices/tree/master/examples) on how to use each of the different devices.


### Usage
To use this library you'll need to `#include <TextDevices.h>` in your sketch.
This gives you the basic `PinsDevice` interface.

```cpp
#include <TextDevices.h>

TextDevices::Devices devices;

void setup() {
    Serial.begin(9600);
    devices.setup(&Serial);
}

void loop() {
    devices.loop();
}
```

Then in the serial console you can do something like this:

```
pin a0 read
PIN a00 112
pin d0 config digital output
pin d0 write 1
```

See the [examples directory](https://github.com/drewfish/arduino-textdevices/tree/master/examples) for details on how to use the other devices.


### Supported Arduinos
This library has been tested on the following arduinos:

* Arduino Uno R3
* [Arduino Pro Mini 328 5V/16Mhz](https://www.sparkfun.com/products/11113)


### Writing Your Own Devices
TODO -- There will be a separate wiki page describing how to do this.


### License

This library is licensed under the MIT license.
See the [LICENSE](https://github.com/drewfish/arduino-textdevices/tree/master/LICENSE) file for more details.


### Contributing to this Library
See the [CONTRIBUTING.md](https://github.com/drewfish/arduino-textdevices/tree/master/CONTRIBUTING.md) file for more details.


