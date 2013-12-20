//
// This mocks out the Arduino API, logging calls that change the state of
// the device.
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
//TODO vector< deque<bool> >   Arduino_digitalRead(14);
//TODO vector< deque<int> >    Arduino_analogRead(6);
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



//-----------------------------------------------------------------------
// Digital I/O
//-----------------------------------------------------------------------

void
pinMode(uint8_t pin, uint8_t mode) {
    char buffer[128];
    snprintf(buffer, 128, "ARDUINO-- pinMode(%d,%d)", pin, mode);
    Arduino_changes.push_back(string(buffer));
}


void
digitalWrite(uint8_t pin, uint8_t value) {
    char buffer[128];
    snprintf(buffer, 128, "ARDUINO-- digitalWrite(%d,%d)", pin, value);
    Arduino_changes.push_back(string(buffer));
}


int
digitalRead(uint8_t pin) {
    // TODO
    return 0;
}



//-----------------------------------------------------------------------
// Analog I/O
//-----------------------------------------------------------------------
// FUTURE
//      void analogReference(uint8_t mode)

int
analogRead(uint8_t pin) {
    // TODO
    return 0;
}


void
analogWrite(uint8_t pin, int value) {
    // TODO
}



//-----------------------------------------------------------------------
// Advanced I/O
//-----------------------------------------------------------------------
// FUTURE
//      void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0)
//      void noTone(uint8_t _pin)
//      void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
//      uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
//      unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L)



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

// TODO
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
// FUTURE
//      lowByte()
//      highByte()
//      bitRead()
//      bitWrite()
//      bitSet()
//      bitClear()
//      bit()



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
    // TODO -- clear Arduino_digitalRead
    // TODO -- clear Arduino_analogRead
    vector<int>().swap(Arduino_Serial_input);
    vector<string>().swap(Arduino_changes);
    vector<string>().swap(Serial.output);
}



#endif
