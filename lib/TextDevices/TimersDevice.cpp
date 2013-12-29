#include "TimersDevice.h"
namespace TextDevices {


    const char*
    TimersDevice::getDeviceName() {
        return "shifters";
    }


    void
    TimersDevice::deviceRegistered(API*, Command*) {
        // nothing to do
    }


    void
    TimersDevice::poll(API*, Command*, uint32_t) {
        // TODO
    }


    bool
    TimersDevice::dispatch(API* api, Command* command) {
        // TODO
        return false;
    }


}
