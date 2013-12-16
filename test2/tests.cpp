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

using namespace std;
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
    void teardown() {
        // TODO -- so we don't leak memory ?
        Arduino_reset();
    }
};


TEST(PinsDevice, digital_config_input) {
    Devices devices;
    devices.setup(&Serial);
    Arduino_set_input("pin d0 config input\n");
    devices.loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "no changes change");
}


TEST(PinsDevice, digital_config_output) {
    Devices devices;
    devices.setup(&Serial);
    Arduino_set_input("pin d0 config output\n");
    devices.loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "just one change");
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[0].c_str());
}



//-----------------------------------------------------------------------
// execution start
//-----------------------------------------------------------------------

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


