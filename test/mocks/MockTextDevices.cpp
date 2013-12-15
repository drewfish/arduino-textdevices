#include "TextDevices.h"
using namespace TextDevices;

class _Devices;

//-----------------------------------------------------------------------
// RawPin class
//-----------------------------------------------------------------------

bool
RawPin::setInput(bool) {
    // TODO
    return false;
}

bool
RawPin::setPullup(bool) {
    // TODO
    return false;
}

bool
RawPin::setType(PinType) {
    // TODO
    return false;
}

bool
RawPin::canRead() {
    // TODO
    return false;
}

bool
RawPin::canWrite() {
    // TODO
    return false;
}

uint32_t
RawPin::rawRead() {
    // TODO
    return 0;
}

bool
RawPin::rawWrite(uint32_t) {
    // TODO
    return false;
}



//-----------------------------------------------------------------------
// API class
//-----------------------------------------------------------------------

API::API(_Devices* d) {
    // TODO
}

RawPin*
API::getRawPin(Command* command, size_t idx) {
    // TODO
    return NULL;
}

RawPin*
API::getRawPin(Command*, const char*) {
    // TODO
    return NULL;
}

bool
API::claimPin(Command*, RawPin*) {
    // TODO
    return false;
}

bool
API::unclaimPin(Command*, RawPin*) {
    // TODO
    return false;
}

bool
API::dispatch(Command*) {
    // TODO
    return false;
}

void
println(Command*, const char*) {
    // TODO
}

bool
error(Command*, const char*) {
    // TODO
    return false;
}

PinsDevice*
getPinsDevice(Command*) {
    // TODO
    return NULL;
}



//-----------------------------------------------------------------------
// Devices class
//-----------------------------------------------------------------------

Devices::Devices() {
    // TODO
}

Devices::~Devices() {
    // TODO
}

void
Devices::setup(Stream*) {
    // TODO
}

void
Devices::registerDevice(IDevice*) {
    // TODO
}

void
Devices::loop() {
    // TODO
}


