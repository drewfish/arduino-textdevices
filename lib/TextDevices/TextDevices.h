#ifndef TEXT_DEVICES_H
#define TEXT_DEVICES_H


#include <Arduino.h>


#define TEXTDEVICES_PINCOUNT NUM_DIGITAL_PINS


namespace TextDevices {

    class API;
    struct Command;
    class IDevice;
    class Devices;
    class _Devices; // internal implementation of Devices class
    class PinsDevice;

    typedef enum { DIGITAL, ANALOG } PinType;


    // management of the physical pin
    struct RawPin {
        uint8_t     idx;        // index in arrays
        uint8_t     hwPin;      // digital pin number (which analog pins can be as well)
        char        id[4];      // textual identifier
        PinType     idType;     // base type of the pin
        uint8_t     idNum;      // number for the type
        PinType     ioType;     // how the pin is configured
        bool        ioInput;    // whether the pin is configured for input
        IDevice*    claimant;   // device which has a claim on the pin
        // be sure to check ioInput before using either of these
        uint32_t    rawRead();
        void        rawWrite(uint32_t);
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
            // given the pin number, returns the raw pin object
            // if invalid id, reports error and returns null
            RawPin* getRawPin(Command*, size_t);

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
            void println(Command*, const char*);

            // reports an error to the user
            // automatically includes original command
            // always returns true
            bool error(Command*, const char*);
    };


    struct Command {
        const char* original;   // the original command as given by the user
        const char* body;       // the command as relative to the device
        IDevice*    device;     // the device handling this command
        bool        hasError;   // whether an error was reported for this command
    };


    // interface which each device should implement
    class IDevice {
        public:
            virtual ~IDevice() {};

            // return a textual identification of the device
            virtual const char* getDeviceName() = 0;

            // called when the device is registered
            virtual void deviceRegistered(API*, Command*) = 0;

            // called each time through loop()
            virtual void poll(API*, Command*, uint32_t) = 0;

            // ask the device to attempt to dispatch the command
            // if the device dispatches the command it should return true
            virtual bool dispatch(API*, Command*) = 0;
    };


    class Devices {
        private:
            _Devices*   _d;
            API*        api;

        public:
            Devices();
            ~Devices();

            void setup(Stream*);

            void registerDevice(IDevice*);

            void loop();

        private:
            void setupPins();
            void setupDefaultDevices();
    };


}




#endif
