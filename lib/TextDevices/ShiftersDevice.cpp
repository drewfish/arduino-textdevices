#include "ShiftersDevice.h"
namespace TextDevices {


    void
    ShiftersDevice::config(API* api, Command* command, Shifter* shifter) {
        char bufferA[4];
        char bufferB[4];
        char bufferC[4];
        if (3 != sscanf(command->body, "%4s %4s %4s", bufferA, bufferB, bufferC)) {
            api->error(command, "invalid config");
            return;
        }
        if (shifter->pinData && !api->unclaimPin(command, shifter->pinData)) {
            // error already reported
            return;
        }
        if (shifter->pinClock && !api->unclaimPin(command, shifter->pinClock)) {
            // error already reported
            return;
        }

        shifter->pinData = api->getRawPin(command, bufferA);
        shifter->pinClock = api->getRawPin(command, bufferB);
        shifter->bitOrder = (0==strncmp(bufferC, "msb", 3)) ? MSBFIRST : LSBFIRST;
        if (! shifter->pinData) {
            api->error(command, "unknown data pin");
            shifter->pinClock = NULL;
            return;
        }
        if (! shifter->pinClock) {
            api->error(command, "unknown clock pin");
            shifter->pinData = NULL;
            return;
        }
        if (! api->claimPin(command, shifter->pinData)) {
            // error already reported
            return;
        }
        if (! api->claimPin(command, shifter->pinClock)) {
            // error already reported
            return;
        }
    }


    void
    ShiftersDevice::unconfig(API* api, Command* command, Shifter* shifter) {
        if (shifter->pinData && !api->unclaimPin(command, shifter->pinData)) {
            // error already reported
            return;
        }
        shifter->pinData = NULL;
        if (shifter->pinClock && !api->unclaimPin(command, shifter->pinClock)) {
            // error already reported
            return;
        }
        shifter->pinClock = NULL;
    }


    void
    ShiftersDevice::in(API* api, Command* command, Shifter* shifter) {
        size_t count = 0;
        uint8_t val;
        char buffer[16];
        if (1 != sscanf(command->body, "%lu", &count)) {
            return;
        }
        if (! count) {
            return;
        }
        if (!shifter->pinData || !shifter->pinClock) {
            api->error(command, "shifter not yet configured");
            return;
        }
        snprintf(buffer, 16, "SHIFT %u IN ", shifter->id);
        api->print(command, buffer);
        for (; count >= 1; count--) {
            val = shiftIn(shifter->pinData->hwPin, shifter->pinClock->hwPin, shifter->bitOrder);
            snprintf(buffer, 16, (count==1 ? "%02X" : "%02X,"), val);
            api->print(command, buffer);
        }
        api->println(command, "");
    }


    void
    ShiftersDevice::out(API* api, Command* command, Shifter* shifter) {
        int val;
        int offset = 0;
        while (sscanf(command->body, "%02x%n", &val, &offset), offset) {
            command->body += offset;
            offset = 0;
            if (',' == command->body[offset]) {
                command->body += 1;
            }
            shiftOut(shifter->pinData->hwPin, shifter->pinClock->hwPin, shifter->bitOrder, val);
        }
    }


    const char*
    ShiftersDevice::getDeviceName() {
        return "shifters";
    }


    void
    ShiftersDevice::deviceRegistered(API*, Command*) {
        memset((void *)this->shifters, 0, sizeof(Shifter) * TEXTDEVICES_SHIFTERCOUNT);
    }


    void
    ShiftersDevice::poll(API*, Command*, uint32_t) {
        // nothing to do
    }


    bool
    ShiftersDevice::dispatch(API* api, Command* command) {
        uint8_t id = 0;
        int offset = 0;
        Shifter *shifter = 0;

        if (1 != sscanf(command->body, "shift %hhu %n", &id, &offset)) {
            return false;
        }
        if (id >= TEXTDEVICES_SHIFTERCOUNT) {
            api->error(command, "invalid shifter id");
            return true;
        }
        shifter = &(this->shifters[id]);
        shifter->id = id;

        command->body += offset;
        offset = 0;

        if (sscanf(command->body, "config %n", &offset), offset) {
            command->body += offset;
            this->config(api, command, shifter);
            return true;
        }
        if (sscanf(command->body, "unconfig %n", &offset), offset) {
            this->unconfig(api, command, shifter);
            return true;
        }
        if (sscanf(command->body, "in %n", &offset), offset) {
            command->body += offset;
            this->in(api, command, shifter);
            return true;
        }
        if (sscanf(command->body, "out %n", &offset), offset) {
            command->body += offset;
            this->out(api, command, shifter);
            return true;
        }

        api->error(command, "unknown command");
        return true;
    }


}
