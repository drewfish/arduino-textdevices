#include "WatchersDevice.h"
namespace TextDevices {


    const char*
    WatchersDevice::getDeviceName() {
        return "shifters";
    }


    void
    WatchersDevice::deviceRegistered(API*, Command*) {
        // nothing to do
    }


    void
    WatchersDevice::poll(API*, Command*, uint32_t) {
        // TODO
    }


    bool
    WatchersDevice::dispatch(API* api, Command* command) {
        // TODO
        return false;
    }


}
