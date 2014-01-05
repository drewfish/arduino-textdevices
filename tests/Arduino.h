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
//
// This mocks out the Arduino API, logging calls that change the state of the device.
//

#ifndef Arduino_h
#define Arduino_h


#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <deque>
#include <string>


using namespace std;



//-----------------------------------------------------------------------
// pins_arduino.h
//-----------------------------------------------------------------------

#define NUM_DIGITAL_PINS 20
#define NUM_ANALOG_INPUTS 6
#define digitalPinHasPWM(p)         ((p) == 3 || (p) == 5 || (p) == 6 || (p) == 9 || (p) == 10 || (p) == 11)



//-----------------------------------------------------------------------
// mock arduino hooks
// (more below)
//-----------------------------------------------------------------------

unsigned long           Arduino_millis;
vector<bool>            Arduino_digitalRead[20];
bool                    Arduino_digitalRead_default = false;
vector<int>             Arduino_analogRead[6];
vector<unsigned long>   Arduino_pulseIn[20];
vector<uint8_t>         Arduino_shiftIn;
vector<int>             Arduino_Serial_input;   // deque was leaking memory for me
vector<string>          Arduino_changes;


void
Arduino_set_input(const char* str) {
    // add chars in reverse order so that we can just pop_back below
    for (size_t i = strlen(str); i; i--) {
        Arduino_Serial_input.push_back(str[i-1]);
    }
}


void
Arduino_changes_reset() {
    vector<string>().swap(Arduino_changes);
}


void
Arduino_changes_dump() {
    cout << "--------------------------------------------- CHANGES " << Arduino_changes.size() << endl;
    for (size_t i = 0; i < Arduino_changes.size(); i++) {
        cout << Arduino_changes[i] << endl;
    }
    cout << "---------------------------------------------" << endl;
}


//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

#define LOW     0x0
#define HIGH    0x1
#define INPUT           0x0
#define OUTPUT          0x1
#define INPUT_PULLUP    0x2
#define LSBFIRST 0
#define MSBFIRST 1



//-----------------------------------------------------------------------
// Digital I/O
//-----------------------------------------------------------------------

void
pinMode(uint8_t pin, uint8_t mode) {
    char buffer[128];
    snprintf(buffer, 128, "ARDUINO-- pinMode(%d,%d)", pin, mode);
    Arduino_changes.push_back(string(buffer));
}


int
digitalRead(uint8_t pin) {
    if (! Arduino_digitalRead[pin].size()) {
        return Arduino_digitalRead_default;
    }
    bool reading = Arduino_digitalRead[pin].front();
    Arduino_digitalRead[pin].erase(Arduino_digitalRead[pin].begin());
    return reading;
}


void
digitalWrite(uint8_t pin, uint8_t value) {
    char buffer[128];
    snprintf(buffer, 128, "ARDUINO-- digitalWrite(%d,%d)", pin, value);
    Arduino_changes.push_back(string(buffer));
}



//-----------------------------------------------------------------------
// Analog I/O
//-----------------------------------------------------------------------
// FUTURE
//      void analogReference(uint8_t mode)

int
analogRead(uint8_t pin) {
    if (! Arduino_analogRead[pin].size()) {
        return 0;
    }
    int reading = Arduino_analogRead[pin].front();
    Arduino_analogRead[pin].erase(Arduino_analogRead[pin].begin());
    return reading;
}


void
analogWrite(uint8_t pin, int value) {
    char buffer[128];
    snprintf(buffer, 128, "ARDUINO-- analogWrite(%d,%i)", pin, value);
    Arduino_changes.push_back(string(buffer));
}



//-----------------------------------------------------------------------
// Advanced I/O
//-----------------------------------------------------------------------
// FUTURE
//      void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0)
//      void noTone(uint8_t _pin)

unsigned long
pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) {
    unsigned long val = Arduino_pulseIn[pin].front();
    Arduino_pulseIn[pin].erase(Arduino_pulseIn[pin].begin());
    if (val >= timeout) {
        Arduino_millis += timeout;
        return 0;
    }
    Arduino_millis += val;
    return val;
}


uint8_t
shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder) {
    uint8_t val = Arduino_shiftIn.front();
    Arduino_shiftIn.erase(Arduino_shiftIn.begin());
    return val;
}


void
shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
    char buffer[128];
    snprintf(buffer, 128, "ARDUINO-- shiftOut(%d,%d,%d,%d)", dataPin, clockPin, bitOrder, val);
    Arduino_changes.push_back(string(buffer));
}



//-----------------------------------------------------------------------
// Time
//-----------------------------------------------------------------------

unsigned long
millis() {
    return Arduino_millis;
}


void
delay(unsigned long ms) {
    Arduino_millis += ms;
    string s = "ARDUINO-- delay(";
    s += ms;
    s += ")";
    Arduino_changes.push_back(s);
}



//-----------------------------------------------------------------------
// Math
//-----------------------------------------------------------------------
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

// FUTURE
//      long map(long, long, long, long, long)



//-----------------------------------------------------------------------
// Trigonometry
//-----------------------------------------------------------------------
// FUTURE
//      sin()
//      cos()
//      tan()



//-----------------------------------------------------------------------
// Random Numbers
//-----------------------------------------------------------------------
// FUTURE
//      randomSeed()
//      random()



//-----------------------------------------------------------------------
// Bits and Bytes
//-----------------------------------------------------------------------
#define bit(b) (1UL << (b))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// FUTURE
//      lowByte()
//      highByte()



//-----------------------------------------------------------------------
// Communication
// We'll just mock the "Serial" global.
//-----------------------------------------------------------------------

struct Stream {
    // buffer output until println to facilitate testing
    vector<string> output;

    int available() {
        return Arduino_Serial_input.size();
    }

    int read() {
        int i = Arduino_Serial_input.back();
        Arduino_Serial_input.pop_back();
        return i;
    }

    size_t print(const char* msg) {
        this->output.push_back(string(msg));
        return 0;
    }


    size_t println(const char* msg) {
        this->output.push_back(string(msg));
        string s = "SERIAL-- ";
        for (size_t i = 0; i < this->output.size(); i++) {
            s += output[i];
        }
        this->output.clear();
        Arduino_changes.push_back(s);
        return 0;
    }

};
Stream Serial;



//-----------------------------------------------------------------------
// mock arduino hooks
// (more above)
//-----------------------------------------------------------------------

void
Arduino_reset() {
    Arduino_millis = 4000;  // in practice the device takes some time to boot
    for (size_t pin = 0; pin < 20; pin++) {
        vector<bool>().swap(Arduino_digitalRead[pin]);
        vector<unsigned long>().swap(Arduino_pulseIn[pin]);
    }
    for (size_t pin = 0; pin < 6; pin++) {
        vector<int>().swap(Arduino_analogRead[pin]);
    }
    vector<uint8_t>().swap(Arduino_shiftIn);
    vector<int>().swap(Arduino_Serial_input);
    Arduino_changes_reset();
    vector<string>().swap(Serial.output);
}



#endif
