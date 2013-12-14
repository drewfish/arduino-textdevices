#include <CppUTest/TestHarness.h>
#include <CppUTest/SimpleString.h>
#include <CppUTest/PlatformSpecificFunctions.h>
#include <CppUTest/CommandLineTestRunner.h>

#include "PinsDevice.h"
#include "PinsDevice.cpp"

using namespace TextDevices;


// http://cpputest.github.io/manual.html#assertions


TEST_GROUP(PinsDevice) {
    void setup() {
    }
    void teardown() {
    }
};


TEST(PinsDevice, getDeviceName) {
    PinsDevice d;
    STRCMP_EQUAL("pins", d.getDeviceName());
}


int main(int ac, char** av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}


