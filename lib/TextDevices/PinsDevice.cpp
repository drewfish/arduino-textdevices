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
    PinsDevice::deviceRegistered(API* api, Command* command) {
        // nothing to do
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
        uint32_t value;

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
                return this->configureRawPin(api, command, pin, 
                        (bufferA[0] == 'd' ? DIGITAL : ANALOG),
                        (bufferB[0] == 'i'),
                        (*(command->body) == 'p')
                );
            }
            // got "config" but it was empty
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
            snprintf(bufferA, 16, "PIN %3s %u", pin->id, value);
            api->println(command, bufferA);
            return true;
        }

        // FUTURE -- support low/off/high/on
        if (1 == sscanf(command->body, "write %u", &value)) {
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

        //DEBUG cout << "---DEBUG--- fell off end --[" << command->body << "]--" << endl;
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
