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


Devices* devices;


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
        devices = new Devices();
        devices->setup(&Serial);
    }
    void teardown() {
        Arduino_reset();
        delete devices;
        devices = NULL;
    }
};


TEST(PinsDevice, digitalpin_config) {
    Arduino_set_input(
            "pin d0 config digital input\n"
            "pin d0 config digital input pullup\n"
            "pin d0 config digital input\n"
            "pin d0 config digital output\n"
            "pin d0 config digital input\n"
            "pin d3 config analog input\n"
            "pin d3 config analog output\n"
            "pin d3 config digital input\n"
    );
    devices->loop();
    CHECK_TEXT(7 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,2)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR digital pin can't be configured for analog input FROM pins WHEN pin d3 config analog input", Arduino_changes[5].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(3,0)", Arduino_changes[6].c_str());
}


TEST(PinsDevice, digitalpin_read) {
    Arduino_set_input(
            "pin d0 read\n"
            "pin d0 read\n"
            "pin d13 read\n"
            "pin d2 config digital write\n"
            "pin d2 read\n"
            "pin d222 read\n"
            "pin 16 read\n"
    );
    Arduino_digitalRead[0].push_back(true);
    Arduino_digitalRead[0].push_back(false);
    Arduino_digitalRead[13].push_back(true);
    devices->loop();
    CHECK_TEXT(7 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("SERIAL-- PIN d00 1", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- PIN d00 0", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- PIN d13 1", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(2,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to read FROM pins WHEN pin d2 read", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin FROM pins WHEN pin d222 read", Arduino_changes[5].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin FROM pins WHEN pin 16 read", Arduino_changes[6].c_str());
}


TEST(PinsDevice, digitalpin_write) {
    Arduino_set_input(
            "pin d0 write 1\n"
            "pin d0 config digital output\n"
            "pin d0 write 1\n"
            "pin d0 write 100\n"
            "pin d0 write 0\n"
            "pin d0 write -10\n"
            "pin d0 config analog output\n"
            "pin d3 config analog output\n"
            "pin d3 write 1\n"
            "pin d3 write 100\n"
            "pin d3 write 300\n"
            "pin d3 write 0\n"
            "pin d3 write -10\n"
    );
    Arduino_digitalRead[0].push_back(true);
    Arduino_digitalRead[0].push_back(false);
    Arduino_digitalRead[13].push_back(true);
    devices->loop();
    CHECK_TEXT(12 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write FROM pins WHEN pin d0 write 1", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[5].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) FROM pins WHEN pin d0 config analog output", Arduino_changes[6].c_str());
    STRCMP_EQUAL("ARDUINO-- analogWrite(3,1)", Arduino_changes[7].c_str());
    STRCMP_EQUAL("ARDUINO-- analogWrite(3,100)", Arduino_changes[8].c_str());
    STRCMP_EQUAL("ARDUINO-- analogWrite(3,255)", Arduino_changes[9].c_str());
    STRCMP_EQUAL("ARDUINO-- analogWrite(3,0)", Arduino_changes[10].c_str());
    STRCMP_EQUAL("ARDUINO-- analogWrite(3,255)", Arduino_changes[11].c_str());
}


// TODO -- more tests


//-----------------------------------------------------------------------
// execution start
//-----------------------------------------------------------------------

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


