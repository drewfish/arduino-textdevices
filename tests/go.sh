#!/usr/bin/env bash

SRCDIR=".."
CPPTEST_HOME="/usr/local/Cellar/cpputest/3.5"   # brew install ccputest

CC=g++
CPPFLAGS="-I${CPPTEST_HOME}/include -I. -I${SRCDIR}"
LD_LIBRARIES="-L${CPPTEST_HOME}/lib -lCppUTest"

echo compiling tests...
${CC} ${CPPFLAGS} ${LD_LIBRARIES} tests.cpp -o tests || exit $?

echo running tests...
./tests -v

