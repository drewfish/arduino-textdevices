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
#include "PinsDevice.h"
namespace TextDevices {


    //-----------------------------------------------------------------------
    // PinsDevice class
    //-----------------------------------------------------------------------

    const char*
    PinsDevice::getDeviceName() {
        return "pins";
    }


    void
    PinsDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    PinsDevice::dispatch(API* api, Command* command) {
        char bufferA[16];   // a couple of generic buffers
        char bufferB[8];    // (so we don't have to allocate so many)
        int offset = 0;
        uint16_t value;

        if (1 != sscanf(command->body, "pin %3s %n", bufferA, &offset)) {
            return false;
        }
        RawPin *pin = api->getRawPin(command, bufferA);
        if (!pin) {
            // api->getRawPin() will have reported an error already
            return true;
        }
        command->body += offset;
        offset = 0;

        if (sscanf(command->body, "config %n", &offset), offset) {
            command->body += offset;
            offset = 0;
            if (sscanf(command->body, "%8s %8s %n", bufferA, bufferB, &offset), offset) {
                command->body += offset;
                this->configureRawPin(api, command, pin, 
                        (bufferA[0] == 'd' ? DIGITAL : ANALOG),
                        (bufferB[0] == 'i'),
                        (*(command->body) == 'p')
                );
                return true;
            }
            // got "config" but it was empty/malformed
            api->error(command, "malformed command");
            return true;
        }

        if (sscanf(command->body, "read %n", &offset), offset) {
            if (! api->claimPin(command, pin)) {
                // error already reported
                return true;
            }
            if (! pin->ioInput) {
                api->error(command, "pin not configured to read");
                return true;
            }
            value = pin->rawRead();
            snprintf(bufferA, 16, "PIN %3s %hu", pin->id, value);
            api->println(command, bufferA);
            return true;
        }

        // FUTURE -- support low/off/high/on
        if (1 == sscanf(command->body, "write %hu", &value)) {
            if (! api->claimPin(command, pin)) {
                // error already reported
                return true;
            }
            if (pin->ioInput) {
                api->error(command, "pin not configured to write");
                return true;
            }
            pin->rawWrite(value);
            return true;
        }

        return false;
    }


    bool
    PinsDevice::configureRawPin(API* api, Command* command, RawPin* pin, PinType type, bool input, bool pullup) {
        // MODES
        //      D,do    valid
        //      D,di    valid
        //      D,ao    depending on PWM ability (digitalPinHasPWM())
        //      D,ai    INVALID
        //      A,do    valid
        //      A,di    valid
        //      A,ao    INVALID (which digitalPinHasPWM() will report)
        //      A,ai    valid, but CAN'T BE CONFIGURED to this mode
        // to summarize
        //      ->do --- valid
        //      ->di --- valid
        //      ->ao --- check digitalPinHasPWM()
        //      ->ai --- INVALID (for different reasons)

        if (! api->claimPin(command, pin)) {
            // error already reported
            return true;
        }
        if (DIGITAL == type) {
            if (input) {
                pinMode(pin->hwPin, pullup ? INPUT_PULLUP : INPUT);
            }
            else {
                pinMode(pin->hwPin, OUTPUT);
            }
        }
        // ANALOG == type
        else {
            if (input) {
                if (DIGITAL == pin->idType) {
                    api->error(command, "digital pin can't be configured for analog input");
                    return false;
                }
                else {
                    if (DIGITAL == pin->ioType) {
                        api->error(command, "analog pin configured to digital is stuck that way");
                        return false;
                    }
                }
            }
            else {
                if (digitalPinHasPWM(pin->hwPin)) {
                    // http://arduino.cc/en/Reference/AnalogWrite
                    // "You do not need to call pinMode() to set the pin as an
                    // output before calling analogWrite()."
                }
                else {
                    api->error(command, "pin doesn't support analog output (PWM)");
                    return false;
                }
            }
        }
        pin->ioType = type;
        pin->ioInput = input;
        return true;
    }


}
