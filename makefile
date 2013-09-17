#!/usr/bin/env make -f

CC = g++
STANDARD = c++0x
OPTIONS = -std=$(STANDARD) -Wall

Assert.h.gch: Assert.h
	$(CC) $(OPTIONS) -x c++-header Assert.h

Assert.o: Assert.cpp Assert.h
	$(CC) $(OPTIONS) -c Assert.cpp

libassert.a: Assert.o
	ar -cvq libassert.a Assert.o

install: libassert.a Assert.h.gch
	cp libassert.a /usr/lib/
	cp Assert.h /usr/include

install_local: libassert.a Assert.h.gch
	cp libassert.a /usr/local/lib/
	cp Assert.h /usr/local/include

clean:
	rm -f Assert.o libassert.a Assert.h.gch *~ *.swp

