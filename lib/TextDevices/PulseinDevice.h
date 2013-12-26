#ifndef TEXTDEVICES_PULSEINDEVICE
#define TEXTDEVICES_PULSEINDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class PulseinDevice: public IDevice {
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
