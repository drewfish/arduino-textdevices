SRCDIR 		= ../../lib/TextDevices
CC			= g++
CPPFLAGS 	+= -I$(SRCDIR) -I. -Wall


all: *.o

%.o: %.cpp
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

clean:
	rm *.o

