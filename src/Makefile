# PAT'S comment
# http://pragprog.com/magazines/2011-04/advanced-arduino-hacking
# in arduino's preferences.txt, write upload.verboe = true
# in source code, replace WProgram.h with Arduino.c
# PROGRAMMER was stk500, fuck
# ARD_REV was 22, fuck
# pins_arduino.h couldn't be found so it was added in INC_DIRS
# see boards.txt for BOARD
# tested with arduino 1.8.9

# Your Arduino environment.
ARD_REV = 1.0
ARD_HOME = /home/pkeroulas/arduino-1.8.9
#/Applications/Arduino.app/Contents/Resources/Java
AVR_HOME = $(ARD_HOME)/hardware/tools/avr
ARD_BIN = $(AVR_HOME)/bin
AVRDUDE = $(ARD_BIN)/avrdude
AVRDUDE_CONF = $(AVR_HOME)/etc/avrdude.conf

# Your favorite serial monitor.
MON_CMD = screen
MON_SPEED = 115200
# MON_SPEED = 9600

# Board settings.
BOARD = uno
#uno
PORT = /dev/ttyACM0
PROGRAMMER = arduino

# Where to find header files and libraries.
INC_DIRS = $(ARD_HOME)/hardware/arduino/avr/variants/standard $(ARD_HOME)/hardware/arduino/avr/libraries/SoftwareSerial
#./inc
LIB_DIRS = $(addprefix $(ARD_HOME)/libraries/, $(LIBS))
LIBS =

include Makefile.master
