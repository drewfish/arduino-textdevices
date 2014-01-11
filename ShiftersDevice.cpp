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
#include "ShiftersDevice.h"
namespace TextDevices {


    void
    ShiftersDevice::Shifter::config(API* api, Command* command) {
        char bufferA[4];
        char bufferB[4];
        char bufferC[4];
        if (3 != sscanf_P(command->body, PSTR("%4s %4s %4s"), bufferA, bufferB, bufferC)) {
            api->error(command, F("invalid config"));
            return;
        }
        if (this->pinData && !api->unclaimPin(command, this->pinData)) {
            // error already reported
            return;
        }
        if (this->pinClock && !api->unclaimPin(command, this->pinClock)) {
            // error already reported
            return;
        }

        this->pinData = api->getRawPin(command, bufferA);
        this->pinClock = api->getRawPin(command, bufferB);
        this->bitOrder = (0==strncmp_P(bufferC, PSTR("msb"), 3)) ? MSBFIRST : LSBFIRST;
        if (! this->pinData) {
            api->error(command, F("unknown data pin"));
            this->pinClock = NULL;
            return;
        }
        if (! this->pinClock) {
            api->error(command, F("unknown clock pin"));
            this->pinData = NULL;
            return;
        }
        if (! api->claimPin(command, this->pinData)) {
            // error already reported
            return;
        }
        if (! api->claimPin(command, this->pinClock)) {
            // error already reported
            return;
        }
    }


    void
    ShiftersDevice::Shifter::unconfig(API* api, Command* command) {
        if (this->pinData && !api->unclaimPin(command, this->pinData)) {
            // error already reported
            return;
        }
        this->pinData = NULL;
        if (this->pinClock && !api->unclaimPin(command, this->pinClock)) {
            // error already reported
            return;
        }
        this->pinClock = NULL;
    }


    void
    ShiftersDevice::Shifter::in(API* api, Command* command) {
        size_t count = 0;
        uint8_t val;
        char buffer[16];
        if (1 != sscanf_P(command->body, PSTR("%lu"), &count)) {
            return;
        }
        if (! count) {
            return;
        }
        if (!this->pinData || !this->pinClock) {
            api->error(command, F("shifter not yet configured"));
            return;
        }
        snprintf_P(buffer, 16, PSTR("SHIFT %u IN "), this->id);
        api->print(command, buffer);
        for (; count >= 1; count--) {
            val = shiftIn(this->pinData->hwPin, this->pinClock->hwPin, this->bitOrder);
            snprintf_P(buffer, 16, (count==1 ? PSTR("%02X") : PSTR("%02X,")), val);
            api->print(command, buffer);
        }
        buffer[0] = 0;
        api->println(command, buffer);
    }


    void
    ShiftersDevice::Shifter::out(API* api, Command* command) {
        int val;
        int offset = 0;
        while (sscanf_P(command->body, PSTR("%02x%n"), &val, &offset), offset) {
            command->body += offset;
            offset = 0;
            if (',' == command->body[offset]) {
                command->body += 1;
            }
            shiftOut(this->pinData->hwPin, this->pinClock->hwPin, this->bitOrder, val);
        }
    }


    ShiftersDevice::ShiftersDevice() {
        memset((void *)this->shifters, 0, sizeof(Shifter) * TEXTDEVICES_SHIFTERCOUNT);
    }


    const char*
    ShiftersDevice::getDeviceName() {
        return "shifters";
    }


    void
    ShiftersDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    ShiftersDevice::dispatch(API* api, Command* command) {
        uint8_t id = 0;
        int offset = 0;
        Shifter *shifter = NULL;

        if (1 != sscanf_P(command->body, PSTR("shift %hhu %n"), &id, &offset)) {
            return false;
        }
        if (id >= TEXTDEVICES_SHIFTERCOUNT) {
            api->error(command, F("invalid shifter id"));
            return true;
        }
        shifter = &(this->shifters[id]);
        shifter->id = id;

        command->body += offset;
        offset = 0;

        if (sscanf_P(command->body, PSTR("config %n"), &offset), offset) {
            command->body += offset;
            shifter->config(api, command);
            return true;
        }
        if (sscanf_P(command->body, PSTR("unconfig %n"), &offset), offset) {
            shifter->unconfig(api, command);
            return true;
        }
        if (sscanf_P(command->body, PSTR("in %n"), &offset), offset) {
            command->body += offset;
            shifter->in(api, command);
            return true;
        }
        if (sscanf_P(command->body, PSTR("out %n"), &offset), offset) {
            command->body += offset;
            shifter->out(api, command);
            return true;
        }

        api->error(command, F("unknown command"));
        return true;
    }


}
