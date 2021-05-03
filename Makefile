UNAME := $(shell uname -m)
ifeq ($(UNAME), aarch64)
	CC=gcc
	CXX=g++
else
	CC=aarch64-linux-gnu-gcc
	CXX=aarch64-linux-gnu-g++
endif

all: pine_gestures toggleflash

install:
	mkdir /opt/pine_gestures
	cp ./pine_gestures /usr/bin
	cp ./toggleflash /usr/bin
	cp ./startup_gestures.sh /opt/pine_gestures

pine_gestures:
	$(CXX) -o pine_gestures pine_gestures.cpp

toggleflash:
	$(CXX) -o toggleflash toggleflash.cpp
     
clean:
	rm pine_gestures
	rm toggleflash
