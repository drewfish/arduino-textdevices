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
#include "I2CDevice.h"
#include <Wire.h>
namespace TextDevices {


    void
    I2CDevice::write(API* api, Command* command) {
        int offset;
        uint8_t address;
        byte error;
        uint8_t val;

        if (1 != sscanf_P(command->body, PSTR("%02hhx %n"), &address, &offset)) {
            api->error(command, F("unknown command"));
            return;
        }
        command->body += offset;
        offset = 0;
        if (0 == command->body[offset]) {
            api->error(command, F("no bytes"));
            return;
        }

        Wire.beginTransmission(address);

        while (sscanf_P(command->body, PSTR("%02hhx%n"), &val, &offset), offset) {
            command->body += offset;
            offset = 0;
            if (',' == command->body[offset]) {
                command->body += 1;
            }
            Wire.write(val);
        }

        error = Wire.endTransmission();
        switch (error) {
            case 0:
                api->println(command, F("I2C SUCCESS"));
                break;
            case 1:
                api->error(command, F("data too long to fit in transmit buffer"));
                break;
            case 2:
                api->error(command, F("received NACK on transmit of address"));
                break;
            case 3:
                api->error(command, F("received NACK on transmit of data"));
                break;
            default:
                api->error(command, F("other transmit error"));
        }
    }


    void
    I2CDevice::read(API* api, Command* command) {
        int offset;
        uint8_t address;
        uint8_t count;
        int val;
        char buffer[4];
        if (2 != sscanf_P(command->body, PSTR("%02hhx %hhu %n"), &address, &count, &offset)) {
            api->error(command, F("unknown command"));
            return;
        }
        if (! count) {
            return;
        }
        api->print(command, F("I2C READ "));
        Wire.requestFrom(address, count);
        while (Wire.available()) {
            val = Wire.read();
            snprintf_P(buffer, 4, (count==1 ? PSTR("%02X") : PSTR("%02X,")), val);
            api->print(command, buffer);
            count--;
            if (! count) {
                break;
            }
        }
        buffer[0] = 0;
        api->println(command, buffer);
    }


    void
    I2CDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    I2CDevice::dispatch(API* api, Command* command) {
        int offset = 0;

        sscanf_P(command->body, PSTR("i2c %n"), &offset);
        if (!offset) {
            return false;
        }
        command->body += offset;
        offset = 0;

        if (sscanf_P(command->body, PSTR("begin %n"), &offset), offset) {
            if (! api->claimPin(command, SDA)) {
                // error already reported
                return true;
            }
            if (! api->claimPin(command, SCL)) {
                // error already reported
                return true;
            }
            Wire.begin();
            return true;
        }

        if (sscanf_P(command->body, PSTR("write %n"), &offset), offset) {
            command->body += offset;
            offset = 0;
            this->write(api, command);
            return true;
        }

        if (sscanf_P(command->body, PSTR("read %n"), &offset), offset) {
            command->body += offset;
            offset = 0;
            this->read(api, command);
            return true;
        }

        return false;
    }


}
