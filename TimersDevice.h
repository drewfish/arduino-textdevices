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
#ifndef TEXTDEVICES_TIMERSDEVICE
#define TEXTDEVICES_TIMERSDEVICE


#ifndef TEXTDEVICES_TIMERCOUNT
    // maximum number of timers
    #define TEXTDEVICES_TIMERCOUNT 4
#endif


#include "TextDevices.h"
namespace TextDevices {

    class TimersDevice: public IDevice {
        private:
            struct Timer {
                uint8_t     id;
                uint32_t    interval;
                char*       command;
                uint32_t    times;      // times left
                uint32_t    next;       // when to run the command next
                void config(API*, Command*, uint32_t, const char*);
                void run(API*, Command*, uint32_t);
                void stop(API*, Command*);
                void poll(API*, uint32_t);
            };
            Timer timers[TEXTDEVICES_TIMERCOUNT];
        public:
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
            ~TimersDevice();
    };

}

#endif
