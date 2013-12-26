#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTest/PlatformSpecificFunctions.h>
#include <CppUTest/CommandLineTestRunner.h>
// http://cpputest.github.io/manual.html#assertions

#include "Arduino.h"
#include "TextDevices.h"
#include "TextDevices.cpp"
#include "PinsDevice.cpp"
#include "ShortcutsDevice.cpp"
#include "PulseinDevice.cpp"

using namespace std;
using namespace TextDevices;


Devices* devices;
IDevice* device;


//-----------------------------------------------------------------------
// PulseinDevice class
//-----------------------------------------------------------------------

TEST_GROUP(PulseinDevice) {
    void setup() {
        Arduino_reset();
        devices = new Devices();
        devices->setup(&Serial);
        device = new PulseinDevice();
        devices->registerDevice(device);
    }
    void teardown() {
        Arduino_reset();
        delete devices;
        devices = NULL;
        delete device;
        device = NULL;
    }
};


TEST(PulseinDevice, all) {
    // setup
    Arduino_set_input(
            "pin d1 config digital output\n"
    );
    devices->loop();
    Arduino_reset();

    Arduino_set_input(
            "pulsein d0 1\n"
            "pulsein d0 1 23\n"
            "pulsein d33 1\n"
            "pulsein d1 1\n"
    );
    Arduino_pulseIn[0].push_back(11);
    Arduino_pulseIn[0].push_back(110);
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("SERIAL-- PULSEIN d00 11", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- PULSEIN d00 TIMEOUT", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin FROM pulsein WHEN pulsein d33 1", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin should be configured for digital input FROM pulsein WHEN pulsein d1 1", Arduino_changes[3].c_str());
}



//-----------------------------------------------------------------------
// ShortcutsDevice class
//-----------------------------------------------------------------------

TEST_GROUP(ShortcutsDevice) {
    void setup() {
        Arduino_reset();
        devices = new Devices();
        devices->setup(&Serial);
        device = new ShortcutsDevice();
        devices->registerDevice(device);
    }
    void teardown() {
        Arduino_reset();
        delete devices;
        devices = NULL;
        delete device;
        device = NULL;
    }
};


TEST(ShortcutsDevice, all) {
    // setup
    Arduino_set_input(
            "pin d1 config digital output\n"
    );
    devices->loop();
    Arduino_reset();

    Arduino_set_input(
            "read d0\n"
            "write d1 1\n"
            "pwm d0 100\n"
            "pwm d3 100\n"
    );
    Arduino_digitalRead[0].push_back(true);
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("SERIAL-- PIN d00 1", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(1,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) FROM pins WHEN pwm d0 100", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- analogWrite(3,100)", Arduino_changes[3].c_str());
}



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
            "pin d0 CONFIG digital input pullup\n"
            "pin d0 config digital input\n"
            "pin d0 config digital output\n"
            "pin d0 config digital input\n"
            "pin d3 config analog input\n"
            "pin d3 config analog output\n"
            "pin d3 config digital input\n"
            "pin d3 config\n"
            "pin d0 config analog output\n"
    );
    devices->loop();
    CHECK_TEXT(9 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,2)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR digital pin can't be configured for analog input FROM pins WHEN pin d3 config analog input", Arduino_changes[5].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(3,0)", Arduino_changes[6].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR malformed command FROM pins WHEN pin d3 config", Arduino_changes[7].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) FROM pins WHEN pin d0 config analog output", Arduino_changes[8].c_str());
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


TEST(PinsDevice, analogpin_config) {
    Arduino_set_input(
            "pIn d0 config analog input\n"
            "pIn a0 config analog input\n"
            "pIn a0 config analog output\n"
            "PIN a1 CONFIG digital input pullup\n"
            "pin a1 config digital output\n"
            "pin a1 config digital input\n"
            "pin a1 config analog input\n"
    );
    devices->loop();
    CHECK_TEXT(6 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("SERIAL-- ERROR digital pin can't be configured for analog input FROM pins WHEN pin d0 config analog input", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) FROM pins WHEN pin a0 config analog output", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(15,2)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(15,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(15,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR analog pin configured to digital is stuck that way FROM pins WHEN pin a1 config analog input", Arduino_changes[5].c_str());
}



//-----------------------------------------------------------------------
// TextDevices class
//-----------------------------------------------------------------------

TEST_GROUP(TextDevices) {
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


TEST(TextDevices, casemangling) {
    Arduino_set_input(
            "pin d0 config digital output\n"
            "PIN d0 write 1\n"
            "PIN D00 WRITE 1\n"
            "AAAaaa\n"
            "zzzZZZ\n"
    );
    devices->loop();
    CHECK_TEXT(5 == Arduino_changes.size(), "no changes change");
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN aaaaaa", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN zzzzzz", Arduino_changes[4].c_str());
}



//-----------------------------------------------------------------------
// execution start
//-----------------------------------------------------------------------

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


