#ifndef TEXTDEVICES_SHORTCUTSDEVICE
#define TEXTDEVICES_SHORTCUTSDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class ShortcutsDevice: public IDevice {
        public:
            const char* getDeviceName();
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
