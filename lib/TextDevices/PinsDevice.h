#ifndef TEXTDEVICES_PINSDEVICE
#define TEXTDEVICES_PINSDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class PinsDevice: public IDevice {
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
            bool configureRawPin(API*, Command*, RawPin*, PinType, bool, bool);
    };

}

#endif
