#include "WatchersDevice.h"
namespace TextDevices {


    void
    WatchersDevice::Watcher::config(API* api, Command* command, const char* change, uint32_t settleTimeout) {
        if (! this->pin->ioInput) {
            api->error(command, "pin not configured for read");
            return;
        }
        this->flags = 0;
        if ('c' == change[0] || 'r' == change[0]) {
            bitSet(this->flags, RISE);
        }
        if ('c' == change[0] || 'f' == change[0]) {
            bitSet(this->flags, FALL);
        }
        this->settleTimeout = settleTimeout;
        this->oldValue = 0;
        this->changeTimeout = 0;
        this->start(api, command);
    }


    void
    WatchersDevice::Watcher::start(API* api, Command* command) {
        if (! this->pin) {
            api->error(command, "watcher hasn't been configured");
            return;
        }
        if (! this->pin->ioInput) {
            api->error(command, "pin not configured for read");
            return;
        }
        if (! api->claimPin(command, this->pin)) {
            // error already reported
            return;
        }
        this->oldValue = this->pin->rawRead();
        bitSet(this->flags, STARTED);
    }


    void
    WatchersDevice::Watcher::stop(API* api, Command* command) {
        if (! this->pin) {
            api->error(command, "watcher hasn't been configured");
            return;
        }
        if (! bitRead(this->flags, STARTED)) {
            api->error(command, "watcher not started");
            return;
        }
        if (! api->unclaimPin(command, this->pin)) {
            // error already reported
            return;
        }
        bitClear(this->flags, STARTED);
        this->changeTimeout = 0;
    }


    void
    WatchersDevice::Watcher::poll(API* api, Command* command, uint32_t now) {
        uint16_t newValue;
        char buffer[64];
        if (this->changeTimeout) {
            if (now < this->changeTimeout) {
                // still within timeout
                // just ignore
            }
            else {
                newValue = this->pin->rawRead();
                if ( (bitRead(this->flags, RISE) && (newValue > oldValue)) ||
                     (bitRead(this->flags, FALL) && (newValue < oldValue)) )
                {
                    // "WATCH {pin} {newValue} {time}"
                    snprintf(buffer, 64, "WATCH %s %hu %u", this->pin->id, newValue, now);
                    api->println(command, buffer);
                }
                this->oldValue = newValue;
                this->changeTimeout = 0;
            }
        }
        else {
            newValue = this->pin->rawRead();
            if (newValue != this->oldValue) {
                this->changeTimeout = millis() + this->settleTimeout;
            }
        }
    }


    const char*
    WatchersDevice::getDeviceName() {
        return "watchers";
    }


    void
    WatchersDevice::poll(API* api, Command* command, uint32_t now) {
        for (uint8_t p = 0; p < TEXTDEVICES_PINCOUNT; p++) {
            if (bitRead(this->watchers[p].flags, STARTED)) {
                this->watchers[p].poll(api, command, now);
            }
        }
    }


    bool
    WatchersDevice::dispatch(API* api, Command* command) {
        char pinID[4];
        RawPin *pin;
        int offset = 0;
        Watcher *watcher = NULL;
        char change[8];
        uint32_t settleTimeout = 0;

        if (1 != sscanf(command->body, "watch %4s %n", pinID, &offset)) {
            return false;
        }
        pin = api->getRawPin(command, pinID);
        if (! pin) {
            // error already reported
            return true;
        }
        watcher = &(this->watchers[pin->hwPin]);
        command->body += offset;
        offset = 0;

        if (2 == sscanf(command->body, "config %8s %u", change, &settleTimeout)) {
            watcher->pin = pin;
            watcher->config(api, command, change, settleTimeout);
            return true;
        }

        if (sscanf(command->body, "start %n", &offset), offset) {
            watcher->start(api, command);
            return true;
        }
        if (sscanf(command->body, "stop %n", &offset), offset) {
            watcher->stop(api, command);
            return true;
        }

        api->error(command, "unknown command");
        return true;
    }


}
