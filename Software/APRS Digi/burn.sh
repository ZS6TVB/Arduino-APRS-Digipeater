#!/bin/bash

cd Default

avr-objcopy -j .text -j .data -O ihex extdigi extdigi.hex
avr-objcopy -j .text -j .data -O binary extdigi extdigi.bin

# Update this parameters accordingly to your setup.

avrdude -v -carduino -pATMEGA328P -P/dev/ttyUSB0 -b115200 -D -Uflash:w:extdigi.hex

