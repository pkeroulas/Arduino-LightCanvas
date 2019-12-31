# Overview

Project page:  http://patkarbo.com/Light-Canvas

License: GPLv3

Author: patkarbo@patkarbo.com

# Hardware

Arduino UNO: http://arduino.cc/en/Main/arduinoBoardUno

# Install

Download and install Arduino Software: http://arduino.cc/en/Main/Software

More information about how to avoid Arduino IDE:
http://pragprog.com/magazines/2011-04/advanced-arduino-hacking

Change absolute path in src/Makefile

```
cd src
make all
make upload
```

# Debug

Open the serial port to manually select and troubleshoot visual fx.

```
microcom -p /dev/ttyACM0 -s 57600
```

Then a number [0-7] to select the fx id.

# Development

To design your own effect, edit src/sequence.h. There are some example in the 'IDEA' section.
