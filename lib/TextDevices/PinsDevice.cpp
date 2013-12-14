#include "PinsDevice.h"
namespace TextDevices {


    //-----------------------------------------------------------------------
    // PinDevice class
    //-----------------------------------------------------------------------

    PinDevice::PinDevice() :
        pin(NULL)
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
        // TODO
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
        // TODO
    }


    PinDevice*
    PinsDevice::getPin(uint8_t p) {
        return &(this->pins[p]);
    }


}
