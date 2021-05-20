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
	mkdir -p /opt/pine_gestures
	cp ./pine_gestures /usr/bin
	cp ./toggleflash /usr/bin
	cp ./startup_gestures.sh /opt/pine_gestures
	cp gestures.service /etc/systemd/system/gestures.service
	systemctl enable gestures.service
	systemctl start gestures.service

uninstall:
	systemctl stop gestures.service
	systemctl disable gestures.service
	rm /etc/systemd/system/gestures.service
	rm -r /opt/pine_gestures
	rm /usr/bin/pine_gestures
	rm /usr/bin/toggleflash


pine_gestures:
	$(CXX) -std=c++11 -o pine_gestures pine_gestures.cpp

toggleflash:
	$(CXX) -std=c++11 -o toggleflash toggleflash.cpp
     
clean:
	rm pine_gestures
	rm toggleflash
