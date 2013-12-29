#ifndef TEXTDEVICES_WATCHERSDEVICE
#define TEXTDEVICES_WATCHERSDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class WatchersDevice: public IDevice {
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
