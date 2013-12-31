#include "TimersDevice.h"
namespace TextDevices {


    void
    TimersDevice::Timer::config(API* api, Command* command) {
        int offset = 0;
        this->interval = 0;
        if (1 != sscanf(command->body, "%u %n", &(this->interval), &offset)) {
            api->error(command, "invalid config");
            return;
        }
        command->body += offset;

        // eat leading spaces
        while (command->body[0] && isspace(command->body[0])) {
            command->body++;
        }

        if (! command->body[0]) {
            api->error(command, "invalid config");
            return;
        }
        if (this->command) {
            free((void*) this->command);
            this->command = NULL;
        }
        this->command = (char*) malloc(strlen(command->body) + 1);
        strcpy(this->command, command->body);
        this->times = 0;
        this->next = 0;
    }


    void
    TimersDevice::Timer::run(API* api, Command* command) {
        this->times = 0;
        if (1 != sscanf(command->body, "%u", &(this->times))) {
            api->error(command, "invalid command");
            return;
        }
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


    const char*
    TimersDevice::getDeviceName() {
        return "timers";
    }


    void
    TimersDevice::deviceRegistered(API*, Command*) {
        // nothing to do
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

        if (1 != sscanf(command->body, "timer %hhu %n", &id, &offset)) {
            return false;
        }
        if (id >= TEXTDEVICES_TIMERCOUNT) {
            api->error(command, "invalid timer id");
            return true;
        }
        timer = &(this->timers[id]);
        timer->id = id;

        command->body += offset;
        offset = 0;
        if (sscanf(command->body, "config %n", &offset), offset) {
            command->body += offset;
            timer->config(api, command);
            return true;
        }
        if (sscanf(command->body, "run %n", &offset), offset) {
            command->body += offset;
            timer->run(api, command);
            return true;
        }
        if (sscanf(command->body, "stop %n", &offset), offset) {
            command->body += offset;
            timer->stop(api, command);
            return true;
        }

        api->error(command, "unknown command");
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
