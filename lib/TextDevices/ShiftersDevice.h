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
            };
            Shifter shifters[TEXTDEVICES_SHIFTERCOUNT];
            void config(API*, Command*, Shifter*);
            void unconfig(API*, Command*, Shifter*);
            void in(API*, Command*, Shifter*);
            void out(API*, Command*, Shifter*);
        public:
            const char* getDeviceName();
            void deviceRegistered(API*, Command*);
            void poll(API*, Command*, uint32_t);
            bool dispatch(API*, Command*);
    };

}

#endif
