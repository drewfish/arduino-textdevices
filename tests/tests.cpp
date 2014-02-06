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

#include "Arduino.h"
#include "Wire.h"
#include "TextDevices.h"
#include "TextDevices.cpp"
#include "I2CDevice.cpp"
#include "PulseinDevice.cpp"
#include "ShiftersDevice.cpp"
#include "ShortcutsDevice.cpp"
#include "TimersDevice.cpp"
#include "WatchersDevice.cpp"

#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTest/PlatformSpecificFunctions.h>
#include <CppUTest/CommandLineTestRunner.h>
// http://cpputest.github.io/manual.html#assertions

using namespace std;
using namespace TextDevices;


Devices* devices;
IDevice* device;


//-----------------------------------------------------------------------
// WatchersDevice class
//-----------------------------------------------------------------------

TEST_GROUP(WatchersDevice) {
    void setup() {
        Arduino_reset();
        devices = new Devices();
        devices->setup(&Serial);
        device = new WatchersDevice();
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


TEST(WatchersDevice, config) {
    Arduino_set_input(
            "watch 0\n"
            "watch 0 config\n"
            "watch 0 config d0\n"
            "watch 0 config d0 change\n"
            "watch 0 config d0 change 20\n"
            "pin d1 config digital output\n"
            "watch 1 config d1 change 20\n"
    );
    devices->loop();
    CHECK_TEXT(6 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN watch 0", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid config WHEN watch 0 config", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid config WHEN watch 0 config d0", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid config WHEN watch 0 config d0 change", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(1,1)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured for read WHEN watch 1 config d1 change 20", Arduino_changes[5].c_str());
}


TEST(WatchersDevice, run_no_changes) {
    Arduino_set_input(
            "watch 0 config d0 change 20\n"
    );
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    for (size_t i = 0; i < 20; i++) {
        Arduino_millis += 4;
        devices->loop();
        CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    }
}


TEST(WatchersDevice, run_change_within_timeout) {
    Arduino_set_input(
            "watch 0 config d0 change 13\n"
    );
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_digitalRead_default = true;
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- WATCH d00 1 4020", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

}


// changes happened, but return to oldValue by time changeTimeout triggers
TEST(WatchersDevice, run_resettle_within_timeout) {
    Arduino_set_input(
            "watch 0 config d0 change 12\n"
    );
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_millis += 4;
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
}


TEST(WatchersDevice, stop_start) {
    // failure modes
    Arduino_set_input(
            "watch 0 start\n"
            "watch 0 stop\n"
            "pin d1 config digital output\nwatch 1 config d1 change 20\n"
            "watch 0 config d0 change 20\nwatch 0 stop\nwatch 0 stop\n"
    );
    devices->loop();
    CHECK_TEXT(5 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR watcher hasn't been configured WHEN watch 0 start", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR watcher hasn't been configured WHEN watch 0 stop", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(1,1)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured for read WHEN watch 1 config d1 change 20", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR watcher not started WHEN watch 0 stop", Arduino_changes[4].c_str());
    Arduino_changes_reset();

    // start, make changes, confirm reports of changes
    Arduino_set_input(
            "watch 0 config d0 change 6\n"
            "watch 0 start\n"
    );
    Arduino_digitalRead_default = false;
    devices->loop();
    Arduino_millis += 4;    // 4004
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4008
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4012
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- WATCH d00 1 4012", Arduino_changes[0].c_str());
    Arduino_changes_reset();

    // stop, make changes, confirm no reports made
    Arduino_digitalRead_default = false;
    Arduino_set_input(
            "watch 0 stop\n"
    );
    devices->loop();
    Arduino_millis += 4;    // 4016
    devices->loop();
    Arduino_millis += 4;    // 4020
    devices->loop();
    Arduino_millis += 4;    // 4024
    devices->loop();
    Arduino_millis += 4;    // 4028
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    // start, make changes, confirm reports of changes
    Arduino_set_input(
            "watch 0 start\n"
    );
    Arduino_digitalRead_default = false;
    devices->loop();
    Arduino_millis += 4;    // 4032
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4036
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4040
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- WATCH d00 1 4040", Arduino_changes[0].c_str());
    Arduino_changes_reset();

    // configure (different config), make changes, confirm reports of changes
    Arduino_set_input(
            "watch 0 config d0 falling 9\n"
    );
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4044
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4048
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4052
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4056
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- WATCH d00 0 4056", Arduino_changes[0].c_str());
    Arduino_changes_reset();

    // start, make more changes, confirm reports of changes
    Arduino_set_input(
            "watch 0 start\n"
    );
    devices->loop();

    Arduino_millis += 4;    // 4060
    Arduino_digitalRead_default = true;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4064
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4068
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4072
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4076
    Arduino_digitalRead_default = false;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4080
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4084
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    Arduino_millis += 4;    // 4088
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- WATCH d00 0 4088", Arduino_changes[0].c_str());
    Arduino_changes_reset();

    // timeout doesn't "refire" if looping on same millisecond
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
}



//-----------------------------------------------------------------------
// TimersDevice class
//-----------------------------------------------------------------------

TEST_GROUP(TimersDevice) {
    void setup() {
        Arduino_reset();
        devices = new Devices();
        devices->setup(&Serial);
        device = new TimersDevice();
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


TEST(TimersDevice, config) {
    Arduino_set_input(
            "timer 0\n"
            "timer 0 config\n"
            "timer 0 config 500\n"
            "timer 5 config 500 pin 0 write 1\n"
            "timer 0 config 500 pin 0 write 1\n"
            "timer 0 config 500 pin 0 write 2\n"    // shouldn't leak memory
    );
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN timer 0", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN timer 0 config", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid config WHEN timer 0 config 500", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid timer id WHEN timer 5 config 500 pin 0 write 1", Arduino_changes[3].c_str());
}


TEST(TimersDevice, run_once) {
    Arduino_set_input(
            "timer 0 config 500 pin d0 write 1\n"
            "timer 0 run\n"
            "timer 0 run 1\n"
    );
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN timer 0 run", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 499;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger yet

    Arduino_changes_reset();
    Arduino_millis += 1;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger again
}


TEST(TimersDevice, run_many) {
    Arduino_set_input(
            "timer 0 config 500     pin d0 write 1\n"
            "timer 0 run 3\n"
    );
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_changes_reset();
    Arduino_millis += 499;  // config + 499
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger yet

    Arduino_changes_reset();
    Arduino_millis += 1;    // config + 500
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;    // config + 501
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger again

    Arduino_changes_reset();
    Arduino_millis += 499;  // config + 1000
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;    // config + 1001
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger again

    Arduino_changes_reset();
    Arduino_millis += 499;  // config + 1500
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;    // config + 1501
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger again

    Arduino_changes_reset();
    Arduino_millis += 499;  // config + 2000
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
    // shouldn't trigger yet
}


TEST(TimersDevice, stop_start) {
    Arduino_set_input(
            "timer 0 config 4 pin d0 write 0\n"
            "timer 0 run 1000\n"
    );
    devices->loop();

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 0", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 0", Arduino_changes[0].c_str());

    Arduino_set_input(
            "timer 0 stop\n"
    );
    devices->loop();

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_set_input(
            "timer 0 run 2\n"
    );
    devices->loop();

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 0", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 0", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 4;
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
}


TEST(TimersDevice, multiple_timers) {
    Arduino_set_input(
            "timer 0 config 4 pin d0 write 0\n"
            "timer 1 config 4 pin d0 write 1\n"
            "timer 0 run 1000\n"
    );
    devices->loop();
    Arduino_millis += 2;
    Arduino_set_input(
            "timer 1 run 1000\n"
    );
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 3
    devices->loop();
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    // start + 4
    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 4
    devices->loop();
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 0", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 5
    devices->loop();
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 6
    devices->loop();
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 7
    devices->loop();
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 8
    devices->loop();
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 0", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 9
    devices->loop();
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 10
    devices->loop();
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());

    Arduino_changes_reset();
    Arduino_millis += 1;        // start + 11
    devices->loop();
    devices->loop();
    CHECK_TEXT(0 == Arduino_changes.size(), "wrong number of changes");
}



//-----------------------------------------------------------------------
// ShiftersDevice class
//-----------------------------------------------------------------------

TEST_GROUP(ShiftersDevice) {
    void setup() {
        Arduino_reset();
        devices = new Devices();
        devices->setup(&Serial);
        device = new ShiftersDevice();
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


TEST(ShiftersDevice, shiftin) {
    Arduino_set_input(
            "shift 0 config d0 d1 msb\n"
            "shift 0 in 3\n"
            "shift 0 out 4F,79,21 \n"
            "shift 1 in 3\n"                    // not yet configured
            "shift 55 config d0 d1 msb\n"       // invalid shifter id
            "shift 0 unconfig\n"
            "shift 0 in 3\n"                    // disabled shifter
            "shift 0 config d55 a33 msb\n"      // invalid pins
            "shift 0 config d2\n"               // invalid config
    );
    Arduino_shiftIn.push_back(3);
    Arduino_shiftIn.push_back(13);
    Arduino_shiftIn.push_back(23);
    devices->loop();
    CHECK_TEXT(11 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- SHIFT 0 IN 03,0D,17", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- shiftOut(0,1,1,79)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- shiftOut(0,1,1,121)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- shiftOut(0,1,1,33)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR shifter not yet configured WHEN shift 1 in 3", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid shifter id WHEN shift 55 config d0 d1 msb", Arduino_changes[5].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR shifter not yet configured WHEN shift 0 in 3", Arduino_changes[6].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN shift 0 config d55 a33 msb", Arduino_changes[7].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN shift 0 config d55 a33 msb", Arduino_changes[8].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown data pin WHEN shift 0 config d55 a33 msb", Arduino_changes[9].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR invalid config WHEN shift 0 config d2", Arduino_changes[10].c_str());
}



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
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- PULSEIN d00 11", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- PULSEIN d00 TIMEOUT", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN pulsein d33 1", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin should be configured for digital input WHEN pulsein d1 1", Arduino_changes[3].c_str());
}


//-----------------------------------------------------------------------
// I2CDevice class
//-----------------------------------------------------------------------

TEST_GROUP(I2CDevice) {
    void setup() {
        Arduino_reset();
        devices = new Devices();
        devices->setup(&Serial);
        device = new I2CDevice();
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


TEST(I2CDevice, begin_calls_begin) {
    Arduino_set_input(
            "i2c begin\n"
    );
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- begin()", Arduino_changes[0].c_str());
}


TEST(I2CDevice, read_without_all_args) {
    Arduino_set_input(
            "i2c read\n"
            "i2c read AF\n"
    );
    devices->loop();
    CHECK_TEXT(2 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN i2c read", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN i2c read af", Arduino_changes[1].c_str());
}


TEST(I2CDevice, read_works) {
    Arduino_set_input(
            "i2c read AF 3\n"
    );
    Wire_read.push_back(12);    // 0x0C
    Wire_read.push_back(23);    // 0x17
    Wire_read.push_back(34);    // 0x22
    devices->loop();
    CHECK_TEXT(8 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- requestFrom(AF,3)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- available()", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- read() 0C", Arduino_changes[2].c_str());
    STRCMP_EQUAL("WIRE-- available()", Arduino_changes[3].c_str());
    STRCMP_EQUAL("WIRE-- read() 17", Arduino_changes[4].c_str());
    STRCMP_EQUAL("WIRE-- available()", Arduino_changes[5].c_str());
    STRCMP_EQUAL("WIRE-- read() 22", Arduino_changes[6].c_str());
    STRCMP_EQUAL("SERIAL-- I2C READ 0C,17,22", Arduino_changes[7].c_str());
}


TEST(I2CDevice, write_without_address) {
    Arduino_set_input(
            "i2c write\n"
    );
    devices->loop();
    CHECK_TEXT(1 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN i2c write", Arduino_changes[0].c_str());
}


TEST(I2CDevice, write_no_bytes) {
    Arduino_set_input(
            "i2c write af\n"
            "i2c write af \n"
            "i2c write af     \n"
    );
    devices->loop();
    CHECK_TEXT(3 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR no bytes WHEN i2c write af", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR no bytes WHEN i2c write af ", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR no bytes WHEN i2c write af     ", Arduino_changes[2].c_str());
}


TEST(I2CDevice, write_with_odd_number_of_chars) {
    Arduino_set_input(
            "i2c write af 1\n"
            "i2c write af 123\n"
            "i2c write af 123,45\n"
    );
    devices->loop();
    CHECK_TEXT(15 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(01)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- I2C SUCCESS", Arduino_changes[3].c_str());
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("WIRE-- write(12)", Arduino_changes[5].c_str());
    STRCMP_EQUAL("WIRE-- write(03)", Arduino_changes[6].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[7].c_str());
    STRCMP_EQUAL("SERIAL-- I2C SUCCESS", Arduino_changes[8].c_str());
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[9].c_str());
    STRCMP_EQUAL("WIRE-- write(12)", Arduino_changes[10].c_str());
    STRCMP_EQUAL("WIRE-- write(03)", Arduino_changes[11].c_str());
    STRCMP_EQUAL("WIRE-- write(45)", Arduino_changes[12].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[13].c_str());
    STRCMP_EQUAL("SERIAL-- I2C SUCCESS", Arduino_changes[14].c_str());
}


TEST(I2CDevice, write_handles_error_codes) {
    Wire_transmissionError = 1;
    Arduino_set_input("i2c write af 01\n");
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(01)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR data too long to fit in transmit buffer WHEN i2c write af 01", Arduino_changes[3].c_str());

    Arduino_changes_reset();
    Wire_transmissionError = 2;
    Arduino_set_input("i2c write af 01\n");
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(01)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR received NACK on transmit of address WHEN i2c write af 01", Arduino_changes[3].c_str());

    Arduino_changes_reset();
    Wire_transmissionError = 3;
    Arduino_set_input("i2c write af 01\n");
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(01)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR received NACK on transmit of data WHEN i2c write af 01", Arduino_changes[3].c_str());

    Arduino_changes_reset();
    Wire_transmissionError = 4;
    Arduino_set_input("i2c write af 01\n");
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(01)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR other transmit error WHEN i2c write af 01", Arduino_changes[3].c_str());

    Arduino_changes_reset();
    Wire_transmissionError = 5;
    Arduino_set_input("i2c write af 01\n");
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(01)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR other transmit error WHEN i2c write af 01", Arduino_changes[3].c_str());
}


TEST(I2CDevice, write_works) {
    Arduino_set_input(
            "i2c write af 00\n"
            "i2c write af ff,12\n"
            "i2c write af fedcba\n"
    );
    devices->loop();
    CHECK_TEXT(15 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("WIRE-- write(00)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- I2C SUCCESS", Arduino_changes[3].c_str());
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("WIRE-- write(FF)", Arduino_changes[5].c_str());
    STRCMP_EQUAL("WIRE-- write(12)", Arduino_changes[6].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[7].c_str());
    STRCMP_EQUAL("SERIAL-- I2C SUCCESS", Arduino_changes[8].c_str());
    STRCMP_EQUAL("WIRE-- beginTransmission(AF)", Arduino_changes[9].c_str());
    STRCMP_EQUAL("WIRE-- write(FE)", Arduino_changes[10].c_str());
    STRCMP_EQUAL("WIRE-- write(DC)", Arduino_changes[11].c_str());
    STRCMP_EQUAL("WIRE-- write(BA)", Arduino_changes[12].c_str());
    STRCMP_EQUAL("WIRE-- endTransmission()", Arduino_changes[13].c_str());
    STRCMP_EQUAL("SERIAL-- I2C SUCCESS", Arduino_changes[14].c_str());
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
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- PIN d00 0", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(1,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) WHEN pwm d0 100", Arduino_changes[2].c_str());
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
    CHECK_TEXT(9 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,2)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR digital pin can't be configured for analog input WHEN pin d3 config analog input", Arduino_changes[5].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(3,0)", Arduino_changes[6].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR malformed command WHEN pin d3 config", Arduino_changes[7].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) WHEN pin d0 config analog output", Arduino_changes[8].c_str());
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
    CHECK_TEXT(7 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- PIN d00 1", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- PIN d00 0", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- PIN d13 1", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(2,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to read WHEN pin d2 read", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN pin d222 read", Arduino_changes[5].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN pin 16 read", Arduino_changes[6].c_str());
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
    devices->loop();
    CHECK_TEXT(12 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[5].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) WHEN pin d0 config analog output", Arduino_changes[6].c_str());
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
    CHECK_TEXT(6 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR digital pin can't be configured for analog input WHEN pin d0 config analog input", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) WHEN pin a0 config analog output", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(15,2)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(15,1)", Arduino_changes[3].c_str());
    STRCMP_EQUAL("ARDUINO-- pinMode(15,0)", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR analog pin configured to digital is stuck that way WHEN pin a1 config analog input", Arduino_changes[5].c_str());
}


TEST(PinsDevice, analogpin_read) {
    Arduino_set_input(
            "pin a0 config digital input\n"
            "pin a1 config analog input\n"
            "pin a0 read\n"
            "pin a1 read\n"
    );
    Arduino_digitalRead[14].push_back(true);
    Arduino_analogRead[1].push_back(13);
    devices->loop();
    CHECK_TEXT(3 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("ARDUINO-- pinMode(14,0)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- PIN a00 1", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- PIN a01 13", Arduino_changes[2].c_str());
}


TEST(PinsDevice, analogpin_write) {
    Arduino_set_input(
            "pin a0 config digital output\n"
            "pin a0 write 1\n"
            "pin a1 config analog output\n"
            "pin a1 write 13\n"
    );
    devices->loop();
    CHECK_TEXT(4 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("ARDUINO-- pinMode(14,1)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(14,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin doesn't support analog output (PWM) WHEN pin a1 config analog output", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin a1 write 13", Arduino_changes[3].c_str());
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
    CHECK_TEXT(5 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("ARDUINO-- pinMode(0,1)", Arduino_changes[0].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[1].c_str());
    STRCMP_EQUAL("ARDUINO-- digitalWrite(0,1)", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN aaaaaa", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR unknown command WHEN zzzzzz", Arduino_changes[4].c_str());
}


TEST(TextDevices, weirdinput) {
    Arduino_set_input(
            "\n"
            " \n"
            "  \n"
            " pin d0 write 1\n"
            "  pin d0 write 1\n"
    );
    devices->loop();
    CHECK_TEXT(2 == Arduino_changes.size(), "wrong number of changes");
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin not configured to write WHEN pin d0 write 1", Arduino_changes[1].c_str());
}


TEST(TextDevices, pin_ids) {
    class A : public IDevice {
        public:
            bool dispatch(API*, Command*) { return false; }
            void poll(API* api, Command* command, uint32_t) {
                RawPin* pin;
                pin = api->getRawPin(command, "");
                CHECK_EQUAL(NULL, pin);
                CHECK_EQUAL(1, Arduino_changes.size());
                STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN poll", Arduino_changes[0].c_str());
                Arduino_changes_reset();
            
                pin = api->getRawPin(command, "foo");
                CHECK_EQUAL(NULL, pin);
                CHECK_EQUAL(1, Arduino_changes.size());
                STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN poll", Arduino_changes[0].c_str());
                Arduino_changes_reset();

                pin = api->getRawPin(command, "d-1");
                CHECK_EQUAL(NULL, pin);
                CHECK_EQUAL(1, Arduino_changes.size());
                STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN poll", Arduino_changes[0].c_str());
                Arduino_changes_reset();

                pin = api->getRawPin(command, "-1");
                CHECK_EQUAL(NULL, pin);
                CHECK_EQUAL(1, Arduino_changes.size());
                STRCMP_EQUAL("SERIAL-- ERROR unknown pin WHEN poll", Arduino_changes[0].c_str());
                Arduino_changes_reset();

                pin = api->getRawPin(command, "d0");
                CHECK(pin != NULL);
                pin = api->getRawPin(command, "d00");
                CHECK(pin != NULL);
                pin = api->getRawPin(command, "a0");
                CHECK(pin != NULL);
                pin = api->getRawPin(command, "a00");
                CHECK(pin != NULL);

                pin = api->getRawPin(command, "0");
                CHECK(NULL != pin);
                CHECK(0 == pin->hwPin);
                STRCMP_EQUAL("d00", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(0 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "1");
                CHECK(NULL != pin);
                CHECK(1 == pin->hwPin);
                STRCMP_EQUAL("d01", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(1 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "2");
                CHECK(NULL != pin);
                CHECK(2 == pin->hwPin);
                STRCMP_EQUAL("d02", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(2 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "3");
                CHECK(NULL != pin);
                CHECK(3 == pin->hwPin);
                STRCMP_EQUAL("d03", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(3 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "4");
                CHECK(NULL != pin);
                CHECK(4 == pin->hwPin);
                STRCMP_EQUAL("d04", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(4 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "5");
                CHECK(NULL != pin);
                CHECK(5 == pin->hwPin);
                STRCMP_EQUAL("d05", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(5 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "6");
                CHECK(NULL != pin);
                CHECK(6 == pin->hwPin);
                STRCMP_EQUAL("d06", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(6 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "7");
                CHECK(NULL != pin);
                CHECK(7 == pin->hwPin);
                STRCMP_EQUAL("d07", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(7 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "8");
                CHECK(NULL != pin);
                CHECK(8 == pin->hwPin);
                STRCMP_EQUAL("d08", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(8 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "9");
                CHECK(NULL != pin);
                CHECK(9 == pin->hwPin);
                STRCMP_EQUAL("d09", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(9 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "10");
                CHECK(NULL != pin);
                CHECK(10 == pin->hwPin);
                STRCMP_EQUAL("d10", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(10 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "11");
                CHECK(NULL != pin);
                CHECK(11 == pin->hwPin);
                STRCMP_EQUAL("d11", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(11 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "12");
                CHECK(NULL != pin);
                CHECK(12 == pin->hwPin);
                STRCMP_EQUAL("d12", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(12 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "13");
                CHECK(NULL != pin);
                CHECK(13 == pin->hwPin);
                STRCMP_EQUAL("d13", pin->id);
                CHECK(DIGITAL == pin->idType);
                CHECK(13 == pin->idNum);
                CHECK(DIGITAL == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "a0");
                CHECK(NULL != pin);
                CHECK(14 == pin->hwPin);
                STRCMP_EQUAL("a00", pin->id);
                CHECK(ANALOG == pin->idType);
                CHECK(0 == pin->idNum);
                CHECK(ANALOG == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "a1");
                CHECK(NULL != pin);
                CHECK(15 == pin->hwPin);
                STRCMP_EQUAL("a01", pin->id);
                CHECK(ANALOG == pin->idType);
                CHECK(1 == pin->idNum);
                CHECK(ANALOG == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "a2");
                CHECK(NULL != pin);
                CHECK(16 == pin->hwPin);
                STRCMP_EQUAL("a02", pin->id);
                CHECK(ANALOG == pin->idType);
                CHECK(2 == pin->idNum);
                CHECK(ANALOG == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "a3");
                CHECK(NULL != pin);
                CHECK(17 == pin->hwPin);
                STRCMP_EQUAL("a03", pin->id);
                CHECK(ANALOG == pin->idType);
                CHECK(3 == pin->idNum);
                CHECK(ANALOG == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "a4");
                CHECK(NULL != pin);
                CHECK(18 == pin->hwPin);
                STRCMP_EQUAL("a04", pin->id);
                CHECK(ANALOG == pin->idType);
                CHECK(4 == pin->idNum);
                CHECK(ANALOG == pin->ioType);
                CHECK(true == pin->ioInput);

                pin = api->getRawPin(command, "a5");
                CHECK(NULL != pin);
                CHECK(19 == pin->hwPin);
                STRCMP_EQUAL("a05", pin->id);
                CHECK(ANALOG == pin->idType);
                CHECK(5 == pin->idNum);
                CHECK(ANALOG == pin->ioType);
                CHECK(true == pin->ioInput);

                CHECK_EQUAL(0, Arduino_changes.size());
            }
    };
    A a;
    devices->registerDevice(&a);
    devices->loop();
}


TEST(TextDevices, pin_ownership) {
    class A : public IDevice {
        private:
            const char* id;
            API*        api;
        public:
            A(const char* i) : id(i) {}
            bool dispatch(API*, Command*) { return false; }
            void poll(API* api, Command* command, uint32_t) {
                this->api = api;
            }
            void claim() {
                Command command;
                command.original = "claim";
                command.body = command.original;
                command.device = this;
                if (! this->api->claimPin(&command, this->api->getRawPin(&command, "d0"))) {
                    this->api->error(&command, this->id);
                }
            }
            void unclaim() {
                Command command;
                command.original = "unclaim";
                command.body = command.original;
                command.device = this;
                if (! this->api->unclaimPin(&command, this->api->getRawPin(&command, "d0"))) {
                    this->api->error(&command, this->id);
                }
            }
    };
    A a("a");
    A b("b");
    devices->registerDevice(&a);
    devices->registerDevice(&b);
    devices->loop();
    a.unclaim();
    a.claim();
    b.claim();
    a.unclaim();
    b.claim();
    a.claim();
    b.unclaim();
    CHECK_EQUAL(6, Arduino_changes.size());
    STRCMP_EQUAL("SERIAL-- ERROR device tried to unclaim a pin that it didn't own WHEN unclaim", Arduino_changes[0].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR a WHEN unclaim", Arduino_changes[1].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin already claimed WHEN claim", Arduino_changes[2].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR b WHEN claim", Arduino_changes[3].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR pin already claimed WHEN claim", Arduino_changes[4].c_str());
    STRCMP_EQUAL("SERIAL-- ERROR a WHEN claim", Arduino_changes[5].c_str());
}



//-----------------------------------------------------------------------
// execution start
//-----------------------------------------------------------------------

int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


