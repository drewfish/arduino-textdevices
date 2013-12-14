
SRCDIR 		= ../lib/TextDevices
CPPTESTDIR  = /usr/local/Cellar/cpputest/3.5

CC			= g++
CPPFLAGS 	+= -I$(CPPTESTDIR)/include -Imocks -I$(SRCDIR)
LD_LIBRARIES = -L$(CPPTESTDIR)/lib -lCppUTest -Lmocks
#  -lCppUTestExt


.PHONY: mocks


all: TestPinsDevice
	./TestPinsDevice -v


mocks: mocks/*.o


mocks/*.o: mocks/*.cpp
	cd mocks && make all


TestPinsDevice: mocks TestPinsDevice.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LD_LIBRARIES) -lMockTextDevices.o TestPinsDevice.cpp -o TestPinsDevice


clean:
	cd mocks && make clean
	rm TestPinsDevice


