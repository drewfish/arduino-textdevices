#include "TextDevices.h"
#include "PinsDevice.h"
#include "ShortcutsDevice.h"
namespace TextDevices {


    //-----------------------------------------------------------------------
    // DeviceList class
    // (linked list)
    //-----------------------------------------------------------------------

    struct DeviceList {
        DeviceList* next;
        IDevice*    device;


        DeviceList() : next(NULL), device(NULL) {}
        DeviceList(IDevice* d) : next(NULL), device(d) {}


        // adds device to end of list
        static void
        push(DeviceList** head, IDevice* device) {
            DeviceList** empty = head;
            while (*empty) {
                empty = &((*empty)->next);
            }
            *empty = new DeviceList(device);
        }


        // removes device from list
        // returns true if device was found and removed
        static bool
        remove(DeviceList** head, IDevice* device) {
            // FUTURE
            return false;
        }


        // TODO -- we really only care about leaking this memory while testing
        // (since on-device these objects lives until the device reboots)
        ~DeviceList() {
            if (this->next) {
                delete this->next;
                this->next = NULL;
            }
            delete this->device;
        }

    };



    //-----------------------------------------------------------------------
    // _Devices class
    // internal implementation of public Devices class
    //-----------------------------------------------------------------------

    struct _Devices {
        RawPin      pins[TEXTDEVICES_PINCOUNT];
        DeviceList* registered;    // registered devices
        PinsDevice* pinsDevice;
        Stream*     stream;
        char        streamBuffer[128];
        size_t      streamBufferNext;


        _Devices() :
            registered(NULL),
            pinsDevice(NULL),
            stream(NULL)
        {
            this->streamBuffer[0] = 0;
            this->streamBufferNext = 0;
            this->pinsDevice = new PinsDevice();
        }


        // TODO -- we really only care about leaking this memory while testing
        // (since on-device these objects lives until the device reboots)
        ~_Devices() {
            // owned by the registered device list
            this->pinsDevice = NULL;

            // clear device list
            if (this->registered) {
                delete this->registered;
                this->registered = NULL;
            }
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
                pin->idx = p;
                pin->hwPin = p;
                //FUTURE pin->hwPort = digital_pin_to_port_PGM[pin->hwPin];
                //FUTURE pin->hwMask = digital_pin_to_bit_mask_PGM[pin->hwPin];
                if (p < analogStart) {
                    pin->idType = DIGITAL;
                    pin->idNum = p;
                    snprintf(pin->id, 4, "d%02u", pin->idNum);
                    pin->ioType = DIGITAL;
                    // defaults according to http://arduino.cc/en/Tutorial/DigitalPins
                    pin->ioInput = true;
                    pin->ioPullup = false;
                }
                else {
                    pin->idType = ANALOG;
                    pin->idNum = p - analogStart;
                    snprintf(pin->id, 4, "a%02u", pin->idNum);
                    pin->ioType = ANALOG;
                    pin->ioInput = true;
                    pin->ioPullup = false;
                }
                pin->claimDevice = NULL;
                pin->pinDevice = this->pinsDevice->getPin(pin->idx);
            }
        }


        void
        setupDefaultDevices(API* api, Command* command) {
            // Devices are attempted in the order they are registered.
            // always attempt these first
            this->registerDevice(api, command, this->pinsDevice);
            this->registerDevice(api, command, new ShortcutsDevice());
        }


        void
        registerDevice(API* api, Command* command, IDevice* device) {
            IDevice *oldDevice = command->device;
            command->device = device;
            DeviceList::push(&(this->registered), device);
            device->deviceRegistered(api, command);
            command->device = oldDevice;
        }


    };



    //-----------------------------------------------------------------------
    // RawPin class
    //-----------------------------------------------------------------------

    bool
    RawPin::setInput(bool input) {
        if (input == this->ioInput) {
            // nothing to do
            return true;
        }
        this->ioInput = input;
        if (this->ioInput) {
            if (this->ioPullup) {
                pinMode(this->hwPin, INPUT);
            }
            else {
                pinMode(this->hwPin, INPUT_PULLUP);
            }
        } else {
            pinMode(this->hwPin, OUTPUT);
        }
        return true;
    }


    bool
    RawPin::setPullup(bool pullup) {
        if (pullup == this->ioPullup) {
            // nothing to do
            return true;
        }
        if (! this->ioInput) {
            return false;
        }
        if (ANALOG == this->ioType) {
            if (pullup) {
                // can't set pullup on an analog-configured pin
                return false;
            }
            return true;
        }
        this->ioPullup = pullup;
        if (this->ioInput) {
            pinMode(this->hwPin, this->ioPullup ? INPUT_PULLUP : INPUT);
        }
        return true;
    }


    bool
    RawPin::setType(PinType type) {
        if (type == this->ioType) {
            // nothing to do
            return true;
        }
        if (DIGITAL == this->idType) {
            if (DIGITAL == this->ioType) {
                if (this->ioInput) {
                    // di->ai -- ERROR digital pin can never analog input
                    return false;
                }
                else {
                    // do->ao -- check PWM
                    if(digitalPinHasPWM(this->hwPin)) {
                        // nothing special to initialized PWM
                        return true;
                    }
                    else {
                        // ERROR digital pin doesn't support analogWrite()
                        return false;
                    }
                }
            }
            else {
                // ai->di -- INTERNAL ERROR shouldn't ever get to analog-input state on a digital pin
                // ao->do -- pinMode(this->hwPin, OUTPUT)
                pinMode(this->hwPin, OUTPUT);
                return true;
            }
        }
        // ANALOG == this->idType
        else {
            if (DIGITAL == this->ioType) {
                // di->ai -- ERROR can't undigital an analog pin
                // do->ao -- ERROR can't undigital an analog pin
                return false;
            }
            else {
                this->ioType = type;
                if (this->ioInput) {
                    pinMode(this->hwPin, this->ioPullup ? INPUT_PULLUP : INPUT);
                    return true;
                }
                else {
                    pinMode(this->hwPin, OUTPUT);
                    return true;
                }
            }
        }
        return false;
    }


    bool
    RawPin::canRead() {
        // TODO
        return false;
    }


    bool
    RawPin::canWrite() {
        // TODO
        return false;
    }


    uint32_t
    RawPin::rawRead() {
        // TODO
        return 0;
    }


    bool
    RawPin::rawWrite(uint32_t val) {
        // TODO
        return false;
    }



    //-----------------------------------------------------------------------
    // API class
    // A wrapper around _Devices that provides just the functionality
    // that devices need.
    //-----------------------------------------------------------------------


    API::API(_Devices* d) {
        this->_d = d;
    }


    RawPin*
    API::getRawPin(Command* command, size_t idx) {
        if (idx >= TEXTDEVICES_PINCOUNT) {
            this->error(command, "unknown pin");
            return NULL;
        }
        return &(this->_d->pins[idx]);
    }


    //  \d+     digital pin
    // d\d+     digital pin
    // a\d+     analog pin
    RawPin*
    API::getRawPin(Command* command, const char* id) {
        uint8_t num;
        PinType type;
        if (1 == sscanf(id, "d%hhu", &num)) {
            type = DIGITAL;
        } else if (1 == sscanf(id, "a%hhu", &num)) {
            type = ANALOG;
        } else if (1 == sscanf(id, "%hhu", &num)) {
            type = DIGITAL;
        } else {
            this->error(command, "unknown pin");
            return NULL;
        }
        for (size_t p = 0; p < TEXTDEVICES_PINCOUNT; p++) {
            RawPin *pin = &(this->_d->pins[p]);
            if (pin->idType == type && pin->idNum == num) {
                return pin;
            }
        }
        this->error(command, "unknown pin");
        return NULL;
    }


    bool
    API::claimPin(Command* command, RawPin* pin) {
        if (pin->claimDevice == command->device) {
            // nothing to do
            return true;
        }
        if (pin->claimDevice != pin->pinDevice) {
            char msg[128];
            snprintf(msg, 128, "pin %s already claimed by %s", pin->id, pin->claimDevice->getDeviceName());
            this->error(command, msg);
            return false;
        }
        pin->claimDevice = command->device;
        return true;
    }


    bool
    API::unclaimPin(Command* command, RawPin* pin) {
        pin->claimDevice = pin->pinDevice;
        return false;
    }


    bool
    API::dispatch(Command* command) {
        DeviceList* d = this->_d->registered;
        while (d) {
            command->device = d->device;
            if (d->device->dispatch(this, command)) {
                return true;
            }
            d = d->next;
        }
        return false;
    }


    void
    API::println(Command* command, const char* msg) {
        // TODO -- decide if command automatically has device name prefixed
        //this->_d->stream->print(command->device->getDeviceName());
        //this->_d->stream->print(" ");
        // TODO -- uppercase msg
        this->_d->stream->println(msg);
    }


    bool
    API::error(Command* command, const char* msg) {
        this->_d->stream->print("ERROR ");
        if (msg) {
            this->_d->stream->print(msg);
        }
        if (command) {
            command->hasError = true;
            if (command->device) {
                this->_d->stream->print(" FROM ");
                this->_d->stream->print(command->device->getDeviceName());
            }
            this->_d->stream->print(" WHEN ");
            this->_d->stream->print(command->original);
        }
        this->_d->stream->println("");
        return true;
    }


    PinsDevice*
    API::getPinsDevice(Command* command) {
        return this->_d->pinsDevice;
    }



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
        Command command;
        command.original = "setup";
        command.body = command.original;
        command.device = NULL;
        this->_d->setup(this->api, &command, stream);
    }


    // These aren't registered during setup() but are available afterwards.
    //devices->registerDevice(new WatchersDevice());
    //devices->registerDevice(new TimersDevice(count));
    //devices->registerDevice(new PWMDevice());
    //devices->registerDevice(new PulseinDevice());
    //devices->registerDevice(new ShiftersDevice(count));
    void
    Devices::registerDevice(IDevice* device) {
        Command command;
        command.original = "registerDevice";
        command.body = command.original;
        command.device = NULL;
        this->_d->registerDevice(this->api, &command, device);
    }

    
    void
    Devices::loop() {
        Command command;

        // poll
        uint32_t now = millis();
        command.original = "poll";
        command.body = command.original;
        command.device = NULL;
        command.hasError = false;
        DeviceList* d = this->_d->registered;
        while (d) {
            command.device = d->device;
            d->device->poll(this->api, &command, now);
            d = d->next;
        }

        while (this->_d->stream->available() > 0) {
            char c = char(this->_d->stream->read());

            if ('\n' == c) {
                // dispatch the command
                // TODO -- lowercase buffer
                command.original = this->_d->streamBuffer;
                command.body = command.original;
                command.device = NULL;
                command.hasError = false;
                this->api->dispatch(&command);

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



}
