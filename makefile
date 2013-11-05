#!/usr/bin/env make -f

CC = g++
STANDARD = c++0x
OPTIONS = -std=$(STANDARD) -Wall

all: libassert.a assert_demo

assert_demo: assert_demo.o assert.o
	$(CC) $(OPTIONS) -o assert_demo assert_demo.o assert.o

assert_demo.o: assert_demo.cpp assert.h
	$(CC) $(OPTIONS) -c assert_demo.cpp

assert.o: assert.cpp assert.h
	$(CC) $(OPTIONS) -c assert.cpp

libassert.a: assert.o
	ar -cvq libassert.a assert.o

install: libassert.a assert.h
	cp libassert.a /usr/lib/
	cp assert.h /usr/include

install_local: libassert.a assert.h
	cp libassert.a /usr/local/lib/
	cp assert.h /usr/local/include

clean:
	rm -f assert_demo assert_demo.o assert.o libassert.a *~ *.swp

