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
        char pin[4];
        char newCommand[32];
        uint32_t value;

        if (1 == sscanf(command->original, "read %4s", pin)) {
            snprintf(newCommand, 32, "pin %s read", pin);
            command->body = newCommand;
            return api->dispatch(command);
        }

        if (2 == sscanf(command->original, "write %4s %u", pin, &value)) {
            snprintf(newCommand, 32, "pin %s write %u", pin, value);
            command->body = newCommand;
            return api->dispatch(command);
        }

        if (2 == sscanf(command->original, "pwm %4s %u", pin, &value)) {
            snprintf(newCommand, 32, "pin %s config analog output", pin);
            command->body = newCommand;
            api->dispatch(command);
            if (command->hasError) {
                // error already reported
                return true;
            }
            snprintf(newCommand, 32, "pin %s write %u", pin, value);
            command->body = newCommand;
            return api->dispatch(command);
        }

        return false;
    }


}
