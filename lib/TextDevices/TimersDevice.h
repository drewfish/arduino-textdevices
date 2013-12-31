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
                void config(API*, Command*);
                void run(API*, Command*);
                void stop(API*, Command*);
                void poll(API*, uint32_t);
            };
            Timer timers[TEXTDEVICES_TIMERCOUNT];
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
            ~TimersDevice();
    };

}

#endif
