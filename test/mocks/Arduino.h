#ifndef Arduino_h
#define Arduino_h


#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <iostream>

using namespace std;


#define NUM_DIGITAL_PINS 20
#define NUM_ANALOG_INPUTS 6


class Stream {
    public:

        size_t print(const char* msg) {
            cout << msg;
            return 0;
        }


        size_t println(const char* msg) {
            cout << msg << endl;
            return 0;
        }


};


#endif
