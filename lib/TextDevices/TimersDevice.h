#ifndef TEXTDEVICES_TIMERSDEVICE
#define TEXTDEVICES_TIMERSDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class TimersDevice: public IDevice {
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
