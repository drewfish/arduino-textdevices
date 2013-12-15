#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTest/PlatformSpecificFunctions.h>
#include <CppUTest/CommandLineTestRunner.h>
// http://cpputest.github.io/manual.html#assertions

#include "Arduino.h"
#include "TextDevices.h"
#include "TextDevices.cpp"
#include "ShortcutsDevice.cpp"
#include "PinsDevice.cpp"

using namespace TextDevices;


//-----------------------------------------------------------------------
// ShortcutsDevice class
//-----------------------------------------------------------------------

// TODO


//-----------------------------------------------------------------------
// PinsDevice class
//-----------------------------------------------------------------------

TEST_GROUP(PinsDevice) {
    void setup() {
        Arduino_reset();
    }
};


TEST(PinsDevice, config) {
    Arduino_set_input("pin d0 config input\n");
    Devices devices;
    devices.setup(&Serial);
    devices.loop();

    FAIL("TODO -- figure out tests for PinsDevice");
}



//-----------------------------------------------------------------------
// execution start
//-----------------------------------------------------------------------

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


