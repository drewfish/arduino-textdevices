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
        char pinId[4];
        int offset = 0;
        if (1 != sscanf(command->body, "pin %3s %n", pinId, &offset)) {
            return false;
        }
        RawPin *pin = api->getRawPin(command, pinId);
        if (!pin) {
            // api->getRawPin() will have reported an error already
            return true;
        }
        command->body += offset;
        offset = 0;

        if (sscanf(command->body, "config %n", &offset), offset) {
            command->body += offset;
            offset = 0;
            char sType[8];
            char sIo[8];
            if (sscanf(command->body, "%8s %8s %n", sType, sIo, &offset), offset) {
                command->body += offset;
                offset = 0;
                return this->configureRawPin(api, command, pin, 
                        (sType[0] == 'd' ? DIGITAL : ANALOG),
                        (sIo[0] == 'i'),
                        (*(command->body) == 'p')
                );
            }
            // got "config" but it was empty
            return true;
        }

        // TODO -- read
        //      claim pin || return true
        //      can read || error and return true
        //      read
        //      report
        //      return true
        // TODO -- write
        //      claim pin || return true
        //      can write || error and return true
        //      write
        //      return true


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
