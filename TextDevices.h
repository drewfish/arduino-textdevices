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
#ifndef TEXT_DEVICES_H
#define TEXT_DEVICES_H


#include <Arduino.h>


#define TEXTDEVICES_PINCOUNT NUM_DIGITAL_PINS
#ifndef TEXTDEVICES_DEVICECOUNT
    // maximum number of devices that can be registered
    #define TEXTDEVICES_DEVICECOUNT 8
#endif


namespace TextDevices {

    class IDevice;
    class API;
    class _Devices; // internal implementation of Devices class


    class Devices {
        private:
            _Devices*   _d;
            API*        api;

        public:
            Devices();
            ~Devices();

            // intializes this class
            // call during Arduino setup()
            void setup(Stream*);

            // add a new device for handling text commands
            // call during Arduino setup()
            void registerDevice(IDevice*);

            // handle commands as they come in
            // also for some devices which are time-sensitive
            // call during Arduino loop()
            void loop();
    };


    // represents the user command being dispatched
    struct Command {
        const char* original;   // the original command as given by the user
        const char* body;       // the command as relative to the device
        IDevice*    device;     // the device handling this command
        bool        hasError;   // whether an error was reported while dispatching this command
    };


    // interface which each device should implement
    class IDevice {
        public:
            virtual ~IDevice() {};

            // return a textual identification of the device
            virtual const char* getDeviceName() = 0;

            // called each time through loop()
            virtual void poll(API*, Command*, uint32_t) = 0;

            // ask the device to attempt to dispatch the command
            // if the device dispatches the command it should return true
            virtual bool dispatch(API*, Command*) = 0;
    };


    // management of the physical pin
    typedef enum { DIGITAL, ANALOG } PinType;
    struct RawPin {
        uint8_t     hwPin;      // digital pin number (which analog pins can be as well)
        char        id[4];      // textual identifier
        PinType     idType;     // base type of the pin
        uint8_t     idNum;      // number for the type
        PinType     ioType;     // how the pin is configured
        bool        ioInput;    // whether the pin is configured for input
        IDevice*    claimant;   // device which has a claim on the pin
        // be sure to check ioInput before using either of these
        uint16_t    rawRead();
        void        rawWrite(uint16_t);
    };


    // helper methods for devices
    // Every method takes the currently active command.
    // If the method does something on behalf of a device, the device given
    // in the command is assumed.
    class API {
        private:
            friend class Devices;
            _Devices*   _d;
            API(_Devices*);
        public:
            // given a textual ID for a pin, returns the raw pin object
            // if invalid id, reports error and returns null
            RawPin* getRawPin(Command*, const char* id);

            // attempts to claim the pin for the device handling the command
            // if pin already claimed, reports error and returns false
            bool claimPin(Command*, RawPin*);

            // releases claim to the pin
            bool unclaimPin(Command*, RawPin*);

            // Dispatches a command to all the registered devices,
            // one at a time, until one device handles it.
            // Returns true if the command was handled.
            bool dispatch(Command*);

            // sends output to the user
            void print(Command*, const char*);

            // sends output to the user
            void println(Command*, const char*);

            // reports an error to the user
            // automatically includes original command
            void error(Command*, const char*);
    };


}



#endif
