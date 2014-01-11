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
#include "ShortcutsDevice.h"
namespace TextDevices {


    const char*
    ShortcutsDevice::getDeviceName() {
        return "shortcuts";
    }


    void
    ShortcutsDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    ShortcutsDevice::dispatch(API* api, Command* command) {
        char pin[4];
        char newCommand[32];
        uint16_t value;

        if (1 == sscanf_P(command->original, PSTR("read %4s"), pin)) {
            snprintf_P(newCommand, 32, PSTR("pin %s read"), pin);
            command->body = newCommand;
            return api->dispatch(command);
        }

        if (2 == sscanf_P(command->original, PSTR("write %4s %hu"), pin, &value)) {
            snprintf_P(newCommand, 32, PSTR("pin %s write %hu"), pin, value);
            command->body = newCommand;
            return api->dispatch(command);
        }

        if (2 == sscanf_P(command->original, PSTR("pwm %4s %hu"), pin, &value)) {
            snprintf_P(newCommand, 32, PSTR("pin %s config analog output"), pin);
            command->body = newCommand;
            api->dispatch(command);
            if (command->hasError) {
                // error already reported
                return true;
            }
            snprintf_P(newCommand, 32, PSTR("pin %s write %hu"), pin, value);
            command->body = newCommand;
            return api->dispatch(command);
        }

        return false;
    }


}
