#include "ShiftersDevice.h"
namespace TextDevices {


    void
    ShiftersDevice::Shifter::config(API* api, Command* command) {
        char bufferA[4];
        char bufferB[4];
        char bufferC[4];
        if (3 != sscanf(command->body, "%4s %4s %4s", bufferA, bufferB, bufferC)) {
            api->error(command, "invalid config");
            return;
        }
        if (this->pinData && !api->unclaimPin(command, this->pinData)) {
            // error already reported
            return;
        }
        if (this->pinClock && !api->unclaimPin(command, this->pinClock)) {
            // error already reported
            return;
        }

        this->pinData = api->getRawPin(command, bufferA);
        this->pinClock = api->getRawPin(command, bufferB);
        this->bitOrder = (0==strncmp(bufferC, "msb", 3)) ? MSBFIRST : LSBFIRST;
        if (! this->pinData) {
            api->error(command, "unknown data pin");
            this->pinClock = NULL;
            return;
        }
        if (! this->pinClock) {
            api->error(command, "unknown clock pin");
            this->pinData = NULL;
            return;
        }
        if (! api->claimPin(command, this->pinData)) {
            // error already reported
            return;
        }
        if (! api->claimPin(command, this->pinClock)) {
            // error already reported
            return;
        }
    }


    void
    ShiftersDevice::Shifter::unconfig(API* api, Command* command) {
        if (this->pinData && !api->unclaimPin(command, this->pinData)) {
            // error already reported
            return;
        }
        this->pinData = NULL;
        if (this->pinClock && !api->unclaimPin(command, this->pinClock)) {
            // error already reported
            return;
        }
        this->pinClock = NULL;
    }


    void
    ShiftersDevice::Shifter::in(API* api, Command* command) {
        size_t count = 0;
        uint8_t val;
        char buffer[16];
        if (1 != sscanf(command->body, "%lu", &count)) {
            return;
        }
        if (! count) {
            return;
        }
        if (!this->pinData || !this->pinClock) {
            api->error(command, "shifter not yet configured");
            return;
        }
        snprintf(buffer, 16, "SHIFT %u IN ", this->id);
        api->print(command, buffer);
        for (; count >= 1; count--) {
            val = shiftIn(this->pinData->hwPin, this->pinClock->hwPin, this->bitOrder);
            snprintf(buffer, 16, (count==1 ? "%02X" : "%02X,"), val);
            api->print(command, buffer);
        }
        api->println(command, "");
    }


    void
    ShiftersDevice::Shifter::out(API* api, Command* command) {
        int val;
        int offset = 0;
        while (sscanf(command->body, "%02x%n", &val, &offset), offset) {
            command->body += offset;
            offset = 0;
            if (',' == command->body[offset]) {
                command->body += 1;
            }
            shiftOut(this->pinData->hwPin, this->pinClock->hwPin, this->bitOrder, val);
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
        Shifter *shifter = NULL;

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
            shifter->config(api, command);
            return true;
        }
        if (sscanf(command->body, "unconfig %n", &offset), offset) {
            shifter->unconfig(api, command);
            return true;
        }
        if (sscanf(command->body, "in %n", &offset), offset) {
            command->body += offset;
            shifter->in(api, command);
            return true;
        }
        if (sscanf(command->body, "out %n", &offset), offset) {
            command->body += offset;
            shifter->out(api, command);
            return true;
        }

        api->error(command, "unknown command");
        return true;
    }


}
