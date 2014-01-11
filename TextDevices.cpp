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
#include "TextDevices.h"
#include <avr/pgmspace.h>
namespace TextDevices {


    //-----------------------------------------------------------------------
    // PinsDevice class
    //-----------------------------------------------------------------------
    class PinsDevice: public IDevice {
        public:
            const char*
            getDeviceName() {
                return "pins";
            }


            void
            poll(API*, Command*, uint32_t) {
                // nothing to do
            }


            bool
            dispatch(API* api, Command* command) {
                char bufferA[16];   // a couple of generic buffers
                char bufferB[8];    // (so we don't have to allocate so many)
                int offset = 0;
                uint16_t value;

                if (1 != sscanf_P(command->body, PSTR("pin %3s %n"), bufferA, &offset)) {
                    return false;
                }
                RawPin *pin = api->getRawPin(command, bufferA);
                if (!pin) {
                    // api->getRawPin() will have reported an error already
                    return true;
                }
                command->body += offset;
                offset = 0;

                if (sscanf_P(command->body, PSTR("config %n"), &offset), offset) {
                    command->body += offset;
                    offset = 0;
                    if (sscanf_P(command->body, PSTR("%8s %8s %n"), bufferA, bufferB, &offset), offset) {
                        command->body += offset;
                        this->configureRawPin(api, command, pin, 
                                (bufferA[0] == 'd' ? DIGITAL : ANALOG),
                                (bufferB[0] == 'i'),
                                (*(command->body) == 'p')
                        );
                        return true;
                    }
                    // got "config" but it was empty/malformed
                    api->error(command, F("malformed command"));
                    return true;
                }

                if (sscanf_P(command->body, PSTR("read %n"), &offset), offset) {
                    if (! api->claimPin(command, pin)) {
                        // error already reported
                        return true;
                    }
                    if (! pin->ioInput) {
                        api->error(command, F("pin not configured to read"));
                        return true;
                    }
                    value = pin->rawRead();
                    snprintf_P(bufferA, 16, PSTR("PIN %3s %hu"), pin->id, value);
                    api->println(command, bufferA);
                    return true;
                }

                // FUTURE -- support low/off/high/on
                if (1 == sscanf_P(command->body, PSTR("write %hu"), &value)) {
                    if (! api->claimPin(command, pin)) {
                        // error already reported
                        return true;
                    }
                    if (pin->ioInput) {
                        api->error(command, F("pin not configured to write"));
                        return true;
                    }
                    pin->rawWrite(value);
                    return true;
                }

                return false;
            }


            bool
            configureRawPin(API* api, Command* command, RawPin* pin, PinType type, bool input, bool pullup) {
                // MODES
                //      D,do    valid
                //      D,di    valid
                //      D,ao    depending on PWM ability (digitalPinHasPWM())
                //      D,ai    INVALID
                //      A,do    valid
                //      A,di    valid
                //      A,ao    INVALID (which digitalPinHasPWM() will report)
                //      A,ai    valid, but CAN'T BE CONFIGURED to this mode
                // to summarize
                //      ->do --- valid
                //      ->di --- valid
                //      ->ao --- check digitalPinHasPWM()
                //      ->ai --- INVALID (for different reasons)

                if (! api->claimPin(command, pin)) {
                    // error already reported
                    return true;
                }
                if (DIGITAL == type) {
                    if (input) {
                        pinMode(pin->hwPin, pullup ? INPUT_PULLUP : INPUT);
                    }
                    else {
                        pinMode(pin->hwPin, OUTPUT);
                    }
                }
                // ANALOG == type
                else {
                    if (input) {
                        if (DIGITAL == pin->idType) {
                            api->error(command, F("digital pin can't be configured for analog input"));
                            return false;
                        }
                        else {
                            if (DIGITAL == pin->ioType) {
                                api->error(command, F("analog pin configured to digital is stuck that way"));
                                return false;
                            }
                        }
                    }
                    else {
                        if (digitalPinHasPWM(pin->hwPin)) {
                            // http://arduino.cc/en/Reference/AnalogWrite
                            // "You do not need to call pinMode() to set the pin as an
                            // output before calling analogWrite()."
                        }
                        else {
                            api->error(command, F("pin doesn't support analog output (PWM)"));
                            return false;
                        }
                    }
                }
                pin->ioType = type;
                pin->ioInput = input;
                return true;
            }
    };



    //-----------------------------------------------------------------------
    // _Devices class
    // internal implementation of public Devices class
    //-----------------------------------------------------------------------

    struct _Devices {
        RawPin      pins[TEXTDEVICES_PINCOUNT];
        IDevice*    registered[TEXTDEVICES_DEVICECOUNT];
        PinsDevice  pinsDevice;
        Stream*     stream;
        char        streamBuffer[128];
        size_t      streamBufferNext;


        _Devices() :
            stream(NULL)
        {
            memset((void *)this->registered, 0, sizeof(IDevice*) * TEXTDEVICES_DEVICECOUNT);
            this->streamBuffer[0] = 0;
            this->streamBufferNext = 0;
        }


        void
        setup(API* api, Command* command, Stream *stream) {
            this->stream = stream;
            this->setupRawPins(api, command);
            this->setupDefaultDevices(api, command);
        }


        void
        setupRawPins(API* api, Command* command) {
            size_t p;
            RawPin* pin;
            size_t analogStart = NUM_DIGITAL_PINS - NUM_ANALOG_INPUTS;
            for (p = 0; p < TEXTDEVICES_PINCOUNT; p++) {
                RawPin* pin = &(this->pins[p]);
                pin->hwPin = p;
                //FUTURE pin->hwPort = digital_pin_to_port_PGM[pin->hwPin];
                //FUTURE pin->hwMask = digital_pin_to_bit_mask_PGM[pin->hwPin];
                if (p < analogStart) {
                    pin->idType = DIGITAL;
                    pin->idNum = p;
                    snprintf_P(pin->id, 4, PSTR("d%02u"), pin->idNum);
                    pin->ioType = DIGITAL;
                    // defaults according to http://arduino.cc/en/Tutorial/DigitalPins
                    pin->ioInput = true;
                }
                else {
                    pin->idType = ANALOG;
                    pin->idNum = p - analogStart;
                    snprintf_P(pin->id, 4, PSTR("a%02u"), pin->idNum);
                    pin->ioType = ANALOG;
                    pin->ioInput = true;
                }
                pin->claimant = &(this->pinsDevice);
            }
        }


        void
        setupDefaultDevices(API* api, Command* command) {
            // Devices are attempted in the order they are registered.
            // always attempt these first
            this->registerDevice(api, command, &(this->pinsDevice));
        }


        void
        registerDevice(API* api, Command* command, IDevice* device) {
            IDevice *oldDevice = command->device;
            command->device = device;
            for (size_t d = 0; d < TEXTDEVICES_DEVICECOUNT; d++) {
                if (! this->registered[d]) {
                    this->registered[d] = device;
                    command->device = oldDevice;
                    return;
                }
            }
            api->error(command, F("no room to register device"));
        }


        void
        lowercaseBuffer() {
            for (size_t i = 0; i < this->streamBufferNext; i++) {
                char c = this->streamBuffer[i];
                if (65 <= c && c <= 90) {
                    c += 32;
                    this->streamBuffer[i] = c;
                }
            }
        }


    };



    //-----------------------------------------------------------------------
    // Devices class
    //-----------------------------------------------------------------------

    Devices::Devices() : _d(NULL), api(NULL) {
        this->_d = new _Devices;
        this->api = new API(this->_d);
    }


    Devices::~Devices() {
        if (this->api) {
            delete this->api;
            this->api = NULL;
        }
        if (this->_d) {
            delete this->_d;
            this->_d = NULL;
        }
    }


    void
    Devices::setup(Stream* stream) {
        char buffer[6];
        strncpy_P(buffer, PSTR("setup"), 6);
        Command command;
        command.original = buffer;
        command.body = command.original;
        command.device = NULL;
        this->_d->setup(this->api, &command, stream);
    }


    // These aren't registered during setup() but are available afterwards.
    //devices->registerDevice(new PulseinDevice());
    //devices->registerDevice(new ShiftersDevice(count));
    //devices->registerDevice(new TimersDevice(count));
    //devices->registerDevice(new WatchersDevice());
    void
    Devices::registerDevice(IDevice* device) {
        char buffer[15];
        strncpy_P(buffer, PSTR("registerDevice"), 15);
        Command command;
        command.original = buffer;
        command.body = command.original;
        command.device = NULL;
        this->_d->registerDevice(this->api, &command, device);
    }

    
    void
    Devices::loop() {
        Command command;

        // poll
        char buffer[5];
        strncpy_P(buffer, PSTR("poll"), 5);
        uint32_t now = millis();
        command.original = buffer;
        command.body = command.original;
        command.device = NULL;
        command.hasError = false;
        for (size_t d = 0; d < TEXTDEVICES_DEVICECOUNT; d++) {
            if (this->_d->registered[d]) {
                command.device = this->_d->registered[d];
                command.hasError = false;
                this->_d->registered[d]->poll(this->api, &command, now);
            }
        }

        while (this->_d->stream->available() > 0) {
            char c = char(this->_d->stream->read());
            char *start;

            if ('\n' == c) {
                // dispatch the command
                this->_d->lowercaseBuffer();
                start = this->_d->streamBuffer;
                while (*start && isspace(*start)) {
                    start++;
                }
                if ('\0' != *start) {
                    command.original = start;
                    command.body = command.original;
                    command.device = NULL;
                    command.hasError = false;
                    if (! this->api->dispatch(&command)) {
                        command.device = NULL;
                        this->api->error(&command, F("unknown command"));
                    }
                }

                // reset buffer
                this->_d->streamBuffer[0] = 0;
                this->_d->streamBufferNext = 0;
                continue;
            }

            // buffer the serial input
            this->_d->streamBuffer[this->_d->streamBufferNext] = c;
            this->_d->streamBufferNext++;
            this->_d->streamBuffer[this->_d->streamBufferNext] = 0;
            if (this->_d->streamBufferNext >= 128) {
                // don't overflow buffer!!!
                break;
            }
        }
    }



    //-----------------------------------------------------------------------
    // RawPin class
    //-----------------------------------------------------------------------


    uint16_t
    RawPin::rawRead() {
        if (DIGITAL == this->ioType) {
            return digitalRead(this->hwPin);
        }
        else {
            return analogRead(this->idNum);
        }
    }


    void
    RawPin::rawWrite(uint16_t val) {
        if (DIGITAL == this->ioType) {
            digitalWrite(this->hwPin, val ? HIGH : LOW);
        }
        else {
            val = constrain(val, 0, 255);
            analogWrite(this->idNum, val);
        }
    }



    //-----------------------------------------------------------------------
    // API class
    // A wrapper around _Devices that provides just the functionality
    // that devices need.
    //-----------------------------------------------------------------------


    API::API(_Devices* d) {
        this->_d = d;
    }


    //  \d+     digital pin
    // d\d+     digital pin
    // a\d+     analog pin
    RawPin*
    API::getRawPin(Command* command, const char* id) {
        uint8_t num;
        PinType type;
        if (1 == sscanf_P(id, PSTR("d%hhu"), &num)) {
            type = DIGITAL;
        } else if (1 == sscanf_P(id, PSTR("a%hhu"), &num)) {
            type = ANALOG;
        } else if (1 == sscanf_P(id, PSTR("%hhu"), &num)) {
            type = DIGITAL;
        } else {
            this->error(command, F("unknown pin"));
            return NULL;
        }
        for (size_t p = 0; p < TEXTDEVICES_PINCOUNT; p++) {
            RawPin *pin = &(this->_d->pins[p]);
            if (pin->idType == type && pin->idNum == num) {
                return pin;
            }
        }
        this->error(command, F("unknown pin"));
        return NULL;
    }


    bool
    API::claimPin(Command* command, RawPin* pin) {
        if (pin->claimant == command->device) {
            // nothing to do
            return true;
        }
        if (pin->claimant != &(this->_d->pinsDevice)) {
            this->error(command, F("pin already claimed"));
            return false;
        }
        pin->claimant = command->device;
        return true;
    }


    bool
    API::unclaimPin(Command* command, RawPin* pin) {
        if (pin->claimant != command->device) {
            this->error(command, F("device tried to unclaim a pin that it didn't own"));
            return false;
        }
        pin->claimant = &(this->_d->pinsDevice);
        return true;
    }


    bool
    API::dispatch(Command* command) {
        if (0 == strncmp_P(command->original, PSTR("freeram"), 7)) {
            char buffer[8];
            snprintf_P(buffer, 8, PSTR("%d"), this->freeRam());
            this->println(command, buffer);
            return true;
        }
        for (size_t d = 0; d < TEXTDEVICES_DEVICECOUNT; d++) {
            if (this->_d->registered[d]) {
                command->device = this->_d->registered[d];
                if (this->_d->registered[d]->dispatch(this, command)) {
                    return true;
                }
            }
        }
        return false;
    }


    void
    API::print(Command* command, const char* msg) {
        this->_d->stream->print(msg);
    }


    void
    API::println(Command* command, const char* msg) {
        this->_d->stream->println(msg);
    }


    void
    API::error(Command* command, const char* msg) {
        this->_d->stream->print(F("ERROR "));
        if (msg) {
            this->_d->stream->print(msg);
        }
        if (command) {
            command->hasError = true;
            if (command->device) {
                this->_d->stream->print(F(" FROM "));
                this->_d->stream->print(command->device->getDeviceName());
            }
            this->_d->stream->print(F(" WHEN "));
            this->_d->stream->print(command->original);
        }
        this->_d->stream->println(F(""));
    }


    void
    API::error(Command* command, const __FlashStringHelper* msg) {
        this->_d->stream->print(F("ERROR "));
        if (msg) {
            this->_d->stream->print(msg);
        }
        if (command) {
            command->hasError = true;
            if (command->device) {
                this->_d->stream->print(F(" FROM "));
                this->_d->stream->print(command->device->getDeviceName());
            }
            this->_d->stream->print(F(" WHEN "));
            this->_d->stream->print(command->original);
        }
        this->_d->stream->println(F(""));
    }



}
