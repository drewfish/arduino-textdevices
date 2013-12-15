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

    };



    //-----------------------------------------------------------------------
    // _Device class
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


        ~_Devices() {
            if (this->pinsDevice) {
                delete this->pinsDevice;
                this->pinsDevice = NULL;
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
        // TODO
        return false;
    }


    bool
    RawPin::setPullup(bool pullup) {
        // TODO
        return false;
    }


    bool
    RawPin::setType(PinType type) {
        // TODO
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
        char buffer[128];
        if (idx >= TEXTDEVICES_PINCOUNT) {
            snprintf(buffer, 128, "unknown raw pin %lu", idx);
            this->error(command, buffer);
            return NULL;
        }
        return &(this->_d->pins[idx]);
    }


    RawPin*
    API::getRawPin(Command* command, const char* id) {
        // TODO
        // if invalid id, send error and return null
        return NULL;
    }


    bool
    API::claimPin(Command* command, RawPin* pin) {
        // TODO
        // if pin already claimed, send error and return false
        return false;
    }


    bool
    API::unclaimPin(Command* command, RawPin* pin) {
        // TODO
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
        // TODO -- uppercase everything
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
