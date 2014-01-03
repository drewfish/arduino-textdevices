#ifndef TEXTDEVICES_SHIFTERSDEVICE
#define TEXTDEVICES_SHIFTERSDEVICE


#ifndef TEXTDEVICES_SHIFTERCOUNT
    // maximum number of shifters
    #define TEXTDEVICES_SHIFTERCOUNT 4
#endif


#include "TextDevices.h"
namespace TextDevices {

    class ShiftersDevice: public IDevice {
        private:
            struct Shifter {
                uint8_t id;
                RawPin  *pinData;
                RawPin  *pinClock;
                uint8_t bitOrder;
                void config(API*, Command*);
                void unconfig(API*, Command*);
                void in(API*, Command*);
                void out(API*, Command*);
            };
            Shifter shifters[TEXTDEVICES_SHIFTERCOUNT];
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
