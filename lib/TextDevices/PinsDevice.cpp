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
        //TODO
        //  if command starts with "config"
        //      return this.config
        //  if command is "read"
        //      return this.read
        //  if command is "write {value}
        //      sscanf {value}
        //      return this.write {value}
        //  else
        //      return false
        return false;
    }


    bool
    PinDevice::config(API* api, Command* command, const char* body) {
        //TODO
        //  claim pin || return
        //  if command body has input/in/output/out/input_pullup/pullup/pu
        //      pin setInput || return error can't set pin IO
        //      pin setPullup || return error can't set pin pullup
        //  if command body has digital/d/analog/a
        //      pin setType || return error can't set pin type
        return false;
    }


    bool
    PinDevice::read(API* api, Command* command) {
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
        //TODO
        //  sscanf "pin {pin}" from command body
        //  raw = api.getRawPin {pin} || return
        //  pinDev = pins[raw.idx]
        //  command.body skip "pin {pin}"
        //  command.device = pinDev
        //  return pinDev.dispatch(api, command)
        return false;
    }


    PinDevice*
    PinsDevice::getPin(uint8_t p) {
        return &(this->pins[p]);
    }


}
