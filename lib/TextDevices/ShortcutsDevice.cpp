#include "ShortcutsDevice.h"
namespace TextDevices {


    const char*
    ShortcutsDevice::getDeviceName() {
        return "shortcuts";
    }


    void
    ShortcutsDevice::deviceRegistered(API*, Command*) {
        // nothing to do
    }


    void
    ShortcutsDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    ShortcutsDevice::dispatch(API* api, Command* command) {
        //TODO
        // "read {pin}"
            // dispatch "pin {pin} read" to api.pins
        // "write {pin} {value}"
            // dispatch "pin {pin} write {value}" to api.pins
        // "pwm {pin} {value}"
            // dispatch "pin d{pin} config analog output" to api.pins
            // check for error
            // dispatch "pin d{pin} write {value}" to api.pins
        return false;
    }


}
