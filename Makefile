UNAME := $(shell uname -m)
ifeq ($(UNAME), aarch64)
	CC=gcc
	CXX=g++
else
	CC=aarch64-linux-gnu-gcc
	CXX=aarch64-linux-gnu-g++
endif

all: shakelight

shakelight:
	$(CXX) -o shakelight main.cpp
     
clean:
	rm shakelight
