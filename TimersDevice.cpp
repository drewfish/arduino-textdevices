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
#include "TimersDevice.h"
namespace TextDevices {


    void
    TimersDevice::Timer::config(API* api, Command* command, uint32_t interval, const char* cmd) {
        this->interval = interval;
        // eat leading spaces
        while (cmd[0] && isspace(cmd[0])) {
            cmd++;
        }
        if (! cmd[0]) {
            api->error(command, F("invalid config"));
            return;
        }
        if (this->command) {
            free((void*) this->command);
            this->command = NULL;
        }
        this->command = (char*) malloc(strlen(cmd) + 1);
        strcpy(this->command, cmd);
        this->times = 0;
        this->next = 0;
    }


    void
    TimersDevice::Timer::run(API* api, Command* command, uint32_t times) {
        this->times = times;
        this->next = millis() + this->interval;
    }


    void
    TimersDevice::Timer::stop(API* api, Command* command) {
        this->next = 0;
    }


    void
    TimersDevice::Timer::poll(API* api, uint32_t now) {
        Command c;
        c.original = this->command;
        c.body = this->command;
        c.device = NULL;
        c.hasError = false;
        api->dispatch(&c);
        this->times -= 1;
        if (this->times > 0) {
            this->next += this->interval;
        }
        else {
            this->next = 0;
        }
    }


    void
    TimersDevice::poll(API* api, Command* command, uint32_t now) {
        Timer *timer = NULL;
        for (size_t t = 0; t < TEXTDEVICES_TIMERCOUNT; t++) {
            timer = &(this->timers[t]);
            if (timer->next && (timer->next <= now)) {
                timer->poll(api, now);
            }
        }
    }


    bool
    TimersDevice::dispatch(API* api, Command* command) {
        uint8_t id = 0;
        int offset = 0;
        Timer *timer = NULL;
        uint32_t value = 0;

        if (1 != sscanf_P(command->body, PSTR("timer %hhu %n"), &id, &offset)) {
            return false;
        }
        if (id >= TEXTDEVICES_TIMERCOUNT) {
            api->error(command, F("invalid timer id"));
            return true;
        }
        timer = &(this->timers[id]);
        timer->id = id;

        command->body += offset;
        offset = 0;
        if (sscanf_P(command->body, PSTR("config %u %n"), &value, &offset), offset) {
            timer->config(api, command, value, command->body + offset);
            return true;
        }
        if (1 == sscanf_P(command->body, PSTR("run %u"), &value)) {
            timer->run(api, command, value);
            return true;
        }
        if (sscanf_P(command->body, PSTR("stop %n"), &offset), offset) {
            command->body += offset;
            timer->stop(api, command);
            return true;
        }

        api->error(command, F("unknown command"));
        return true;
    }


    TimersDevice::~TimersDevice() {
        for (size_t t = 0; t < TEXTDEVICES_TIMERCOUNT; t++) {
            if (this->timers[t].command) {
                free((void*) this->timers[t].command);
                this->timers[t].command = NULL;
            }
        }
    }


}
