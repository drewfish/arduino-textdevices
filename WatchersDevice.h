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
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
