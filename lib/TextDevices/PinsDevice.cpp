#include "PinsDevice.h"
namespace TextDevices {


    //-----------------------------------------------------------------------
    // PinDevice class
    //-----------------------------------------------------------------------

    PinDevice::PinDevice() : pin(NULL)
    {
        this->name[0] = 0;
    }


    void
    PinDevice::setup(RawPin* pin) {
        this->pin = pin;
        snprintf(this->name, 8, "pin %s", this->pin->id);
    }


    const char*
    PinDevice::getDeviceName() {
        return this->name;
    }


    void
    PinDevice::deviceRegistered(API* api, Command* command) {
        // nothing to do
    }


    void
    PinDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    PinDevice::dispatch(API* api, Command* command) {
        if (0 == strncmp("config ", command->body, 7)) {
            return this->config(api, command, command->body + 7);
        }
        if (0 == strncmp("read", command->body, 4)) {
            return this->read(api, command);
        }
        uint32_t value;
        if (1 == sscanf(command->body, "write %u", &value)) {
            return this->write(api, command, value);
        }
        return false;
    }


    bool
    PinDevice::config(API* api, Command* command, const char* body) {
        PinType type    = this->pin->ioType;
        bool    input   = this->pin->ioInput;
        bool    pullup  = this->pin->ioPullup;
        char token[16];
        int offset;
        while (1 == sscanf(body, "%16s%n", token, &offset)) {
            body += offset;
            if (0 == strncmp("out", token, 3)) {
                input = false;
                pullup = false;
                continue;
            }
            if (0 == strncmp("in", token, 2)) {
                input = true;
                pullup = false;
                continue;
            }
            if (0 == strncmp("pu", token, 2)) {
                input = true;
                pullup = true;
                continue;
            }
            if (0 == strncmp("d", token, 1)) {
                type = DIGITAL;
                continue;
            }
            if (0 == strncmp("a", token, 1)) {
                type = ANALOG;
                continue;
            }
        }
        if (! this->pin->setType(type)) {
            api->error(command, "can't set type");
            return true;
        }
        if (! this->pin->setInput(input)) {
            api->error(command, "can't set input/output");
            return true;
        }
        if (! this->pin->setPullup(pullup)) {
            api->error(command, "can't set pullup");
            return true;
        }
        return true;
    }


    bool
    PinDevice::read(API* api, Command* command) {
        //cout << "======TODO====== PinDevice::read() --pin=" << this->pin->id << endl;
        //TODO
        //  check if owns pin
        //  set to input, if necessary
        //  restore pullup, if necessary
        //  digitalRead(pin.idNum) or analogRead(pin.idNum)
        //  api.println
        return false;
    }


    bool
    PinDevice::write(API* api, Command* command, uint32_t value) {
        //cout << "======TODO====== PinDevice::config() --pin=" << this->pin->id << " --value" << value << endl;
        //TODO
        //  check if owns pin
        //  check if value constrained to pin.ioType
        //  check if can analogWrite() to pin, if necessary
        //  set to output, if necessary
        //  digitalWrite(pin.idNum, value) or analogWrite(pin.idNum value)
        return false;
    }



    //-----------------------------------------------------------------------
    // PinsDevice class
    //-----------------------------------------------------------------------

    const char*
    PinsDevice::getDeviceName() {
        return "pins";
    }


    void
    PinsDevice::deviceRegistered(API* api, Command* command) {
        for (size_t p = 0; p < TEXTDEVICES_PINCOUNT; p++) {
            RawPin* rawPin = api->getRawPin(command, p);
            if (rawPin) {
                this->pins[p].setup(rawPin);
            }
        }
    }


    void
    PinsDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    PinsDevice::dispatch(API* api, Command* command) {
        char pinId[4];
        int offset;
        if (1 == sscanf(command->body, "pin %3s %n", pinId, &offset)) {
            RawPin *pinRaw = api->getRawPin(command, pinId);
            if (!pinRaw) {
                // api->getRawPin() will have reported an error already
                return true;
            }
            PinDevice *pinDev = &(this->pins[pinRaw->idx]);
            command->body += offset;
            command->device = pinDev;
            return pinDev->dispatch(api, command);
        }
        return false;
    }


    PinDevice*
    PinsDevice::getPin(uint8_t pin) {
        return &(this->pins[pin]);
    }


}
