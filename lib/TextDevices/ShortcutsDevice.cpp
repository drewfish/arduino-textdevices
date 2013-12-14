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
        // TODO
        // "read {pin}"
            // assign to api.pins (PinsDevice)
            // rewrite to "pin {pin} read"
            // dispatch to api.pins
        // "write {pin} {value}"
            // assign to api.pins (PinsDevice)
            // rewrite to "pin {pin} write {value}"
            // dispatch to api.pins
        // "pwm {pin} {value}"
            // assign to api.pins (PinsDevice)
            // rewrite to "pin d{pin} config analog output"
            // dispatch to api.pins
            // check for error
            // rewrite to "pin d{pin} write {value}"
            // dispatch to api.pins
    }


}
