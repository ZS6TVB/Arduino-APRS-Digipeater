extdigi, an APRS Digipeater for Ardiuno.
----------------------------------------

Alejandro Santos, LU4EXT.

For more information visit: 
  http://extradio.sourceforge.net/

BeRTOS is Copyright Develer S.r.l. (http://www.develer.com/).

CONFIGURATION:
--------------

This is somewhat messy, needs to be cleaned up.

Callsign:
- Edit the file src/main.c and change the two variables MYCALL and MYCALL_SSID.

Beacon message:
- Edit the file src/main.c and change the two constants: APRS_BEACON_MSG and 
  APRS_BEACON_TIME.

Preamble length (TXdelay in AGWPE):
- Edit the file inclue/cfg/cfg_afsk.h and change the constant 
  CONFIG_AFSK_PREAMBLE_LEN.

Building:
---------

To build just type:

  make

You'll need the AVR GCC toolchain. In Debian and Ubuntu Linux distributions
install the following packages:

  aptitude install avr-libc avrdude gcc-avr make

To burn in an ATmega328P/Arduino use avrdude:

  avrdude -v -c arduino -p ATMEGA328P -P /dev/ttyUSB0 -b 115200 -D -U flash:w:extdigi.hex

