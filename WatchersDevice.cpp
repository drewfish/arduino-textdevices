/*
The MIT License (MIT)

Copyright (c) 2014 Drew Folta

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include "WatchersDevice.h"
namespace TextDevices {


    void
    WatchersDevice::Watcher::config(API* api, Command* command) {
        char pinID[4];
        char change[8];
        uint32_t settleTimeout = 0;

        if (3 != sscanf_P(command->body, PSTR("%4s %8s %u"), pinID, change, &settleTimeout)) {
            api->error(command, F("invalid config"));
            return;
        }
        this->pin = api->getRawPin(command, pinID);
        if (! this->pin) {
            // error already reported
            return;
        }
        if (! this->pin->ioInput) {
            api->error(command, F("pin not configured for read"));
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
            api->error(command, F("watcher hasn't been configured"));
            return;
        }
        if (! this->pin->ioInput) {
            api->error(command, F("pin not configured for read"));
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
            api->error(command, F("watcher hasn't been configured"));
            return;
        }
        if (! bitRead(this->flags, STARTED)) {
            api->error(command, F("watcher not started"));
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
                    snprintf_P(buffer, 64, PSTR("WATCH %s %hu %u"), this->pin->id, newValue, now);
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


    WatchersDevice::WatchersDevice() {
        memset((void *)this->watchers, 0, sizeof(Watcher) * TEXTDEVICES_WATCHERCOUNT);
    }


    void
    WatchersDevice::poll(API* api, Command* command, uint32_t now) {
        for (uint8_t w = 0; w < TEXTDEVICES_WATCHERCOUNT; w++) {
            if (bitRead(this->watchers[w].flags, STARTED)) {
                this->watchers[w].poll(api, command, now);
            }
        }
    }


    bool
    WatchersDevice::dispatch(API* api, Command* command) {
        uint8_t id = 0;
        int offset = 0;
        Watcher *watcher = NULL;

        if (1 != sscanf_P(command->body, PSTR("watch %hhu %n"), &id, &offset)) {
            return false;
        }
        if (id >= TEXTDEVICES_WATCHERCOUNT) {
            api->error(command, F("invalid watcher id"));
            return true;
        }
        watcher = &(this->watchers[id]);
        watcher->id = id;

        command->body += offset;
        offset = 0;

        if (sscanf_P(command->body, PSTR("config %n"), &offset), offset) {
            command->body += offset;
            watcher->config(api, command);
            return true;
        }
        if (sscanf_P(command->body, PSTR("start %n"), &offset), offset) {
            watcher->start(api, command);
            return true;
        }
        if (sscanf_P(command->body, PSTR("stop %n"), &offset), offset) {
            command->body += offset;
            watcher->stop(api, command);
            return true;
        }

        api->error(command, F("unknown command"));
        return true;
    }


}
