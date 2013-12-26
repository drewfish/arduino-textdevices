#include "PulseinDevice.h"
namespace TextDevices {


    const char*
    PulseinDevice::getDeviceName() {
        return "pulsein";
    }


    void
    PulseinDevice::deviceRegistered(API*, Command*) {
        // nothing to do
    }


    void
    PulseinDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    PulseinDevice::dispatch(API* api, Command* command) {
        char pin[4];
        uint32_t value;
        unsigned long timeout = 1000000L;

        // FUTURE -- support low/off/high/on
        if (sscanf(command->original, "pulsein %4s %u %lu", pin, &value, &timeout) < 2) {
            return false;
        }

        RawPin* raw = api->getRawPin(command, pin);
        if (!raw) {
            // error already reported
            return true;
        }

        if (! api->claimPin(command, raw)) {
            // error already reported
            return true;
        }

        if (DIGITAL != raw->ioType || !raw->ioInput) {
            api->error(command, "pin should be configured for digital input");
            return true;
        }


        value = value ? HIGH : LOW;
        unsigned long time = pulseIn(raw->hwPin, value, timeout);
        char buffer[32];
        if (time) {
            snprintf(buffer, 32, "PULSEIN %s %lu", raw->id, time);
            api->println(command, buffer);
        }
        else {
            snprintf(buffer, 32, "PULSEIN %s TIMEOUT", raw->id);
            api->println(command, buffer);
        }
        return true;
    }


}
