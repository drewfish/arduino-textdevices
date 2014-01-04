#ifndef TEXTDEVICES_WATCHERSDEVICE
#define TEXTDEVICES_WATCHERSDEVICE

#include "TextDevices.h"
namespace TextDevices {

    class WatchersDevice: public IDevice {
        private:
            enum {
                STARTED = 1,    // whether we're watching or not
                RISE,           // whether to report rising value
                FALL,           // whether to report falling value
            };
            struct Watcher {
                RawPin*     pin;
                uint8_t     flags;
                uint32_t    settleTimeout;
                uint16_t    oldValue;
                uint32_t    changeTimeout;  // also indicates substate of "started"
                void config(API*, Command*, const char*, uint32_t);
                void start(API*, Command*);
                void stop(API*, Command*);
                void poll(API*, Command*, uint32_t);
            };
            Watcher watchers[TEXTDEVICES_PINCOUNT];
        public:
            const char* getDeviceName();
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
