#ifndef TEXTDEVICES_PINSDEVICE
#define TEXTDEVICES_PINSDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class PinDevice: public IDevice {
        private:
            char    name[8];
            RawPin  *pin;
        public:
            PinDevice();
            void setup(RawPin*);
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

    class PinsDevice: public IDevice {
        private:
            PinDevice   pins[TEXTDEVICES_PINCOUNT];
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
            PinDevice* getPin(uint8_t);
    };

}

#endif
