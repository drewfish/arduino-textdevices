#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTest/PlatformSpecificFunctions.h>
#include <CppUTest/CommandLineTestRunner.h>
// http://cpputest.github.io/manual.html#assertions

#include "PinsDevice.h"
#include "PinsDevice.cpp"

using namespace TextDevices;


API* api = NULL;
RawPin* rawPin = NULL;



//-----------------------------------------------------------------------
// PinsDevice class
//-----------------------------------------------------------------------

TEST_GROUP(PinsDevice) {
    void setup() {
        if (api) {
            delete api;
            api = NULL;
        }
        api = new API(NULL);
    }
    void teardown() {
        if (api) {
            delete api;
            api = NULL;
        }
    }
};


TEST(PinsDevice, getDeviceName) {
    PinsDevice device;
    STRCMP_EQUAL("pins", device.getDeviceName());
}


TEST(PinsDevice, deviceRegistered) {
    FAIL("TODO -- write test");
}


TEST(PinsDevice, poll) {
    FAIL("TODO -- write test");
}


TEST(PinsDevice, dispatch) {
    FAIL("TODO -- write test");
}


TEST(PinsDevice, getPin) {
    FAIL("TODO -- write test");
}



//-----------------------------------------------------------------------
// PinDevice class
//-----------------------------------------------------------------------

TEST_GROUP(PinDevice) {
    void setup() {
        api = new API(NULL);
        rawPin = new RawPin();
        snprintf(rawPin->id, 4, "r01");
    }
    void teardown() {
        if (api) {
            delete api;
            api = NULL;
        }
        if (rawPin) {
            delete rawPin;
            rawPin = NULL;
        }
    }
};


TEST(PinDevice, ctor) {
    FAIL("TODO -- write test");
}


TEST(PinDevice, setup) {
    FAIL("TODO -- write test");
}


TEST(PinDevice, getDeviceName) {
    PinDevice device;
    device.setup(rawPin);
    STRCMP_EQUAL("pin r01", device.getDeviceName());
}


TEST(PinDevice, deviceRegistered) {
    FAIL("TODO -- write test");
}


TEST(PinDevice, poll) {
    FAIL("TODO -- write test");
}


TEST(PinDevice, dispatch) {
    FAIL("TODO -- write test");
}



//-----------------------------------------------------------------------
// execution start
//-----------------------------------------------------------------------

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


