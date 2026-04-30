# AVR configuration - fuse bits

The hardware configuration of AVR microcontrollers is kept in locations called fuses. Fuses are non-volatile memory locations and can be changed with the use of a downloader application like AVR Dude. Generally, fuses are configured (called burning fuses) when a microcontroller is prepared for programming.

AVRdude - Downloader/uploader

AVRdude is an open-source CLI tool which facilitates reading and writing to memories in an AVR: flash, EEPROM, and configuration fuses. AVRdude supports several programmers and a large number of AVR microcontrollers. A typical format of an AVRdude command is as follows,

avrdude -c <programmer id> -p <part no> -P <port>

-c

We generally use usbasp as the programmer. In fact Arduino can also be used as a programmer

-p

The part number is the device we are going to program. Following are some of the common devices and corresponding part IDs

ATmega32  - m32

ATmega328 - m328

ATmega328p - m328p

-P

In general, we use USB ports hence use usb as the port.

-U

This command line option is used for performing the memory operations.
Fuse value calculations

You can obtain details about the fuse bits of each microcontroller from the datasheet. https://www.engbedded.com/fusecalc/ is a nice fuse calculator tool provides easy to understand interface.

Examples

ATmega328p burning fuse [1 MHz, with enable reset pin ( that is no PC6, )

avrdude -c usbasp -p m328p -P usb -U lfuse:w:0x62:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

ATmega328p burning fuse [8 MHz, with enable reset pin ( that is no PC6, )

avrdude -c usbasp -p m328p -P usb -U lfuse:w:0xe2:m -U hfuse:w:0xd9:m -U efuse:w:0xff:m

ATmega32a - Internal RC oscillator at 1 MHz, JTAG disabled

avrdude -c usbasp -p atmega32 -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m

ATmega32a - Internal RC oscillator at 8 MHz, JTAG disabled

avrdude -c usbasp -p atmega32 -U lfuse:w:0xc4:m -U hfuse:w:0xd9:m

Slow devices

Recently we have noticed that there are slow devices avaible as usbasp hardware. In that case you need to reduce the baudrate by adding "usb -B 5.33".

E.g.

avrdude -c usbasp usb -B 5.33  -p atmega32 -U lfuse:w:0xe1:m -U hfuse:w:0xd9:m

In case of slow devices you need to use the following code for downloading the hex file

avrdude -p m32 usb -B 5.33 -c usbasp -e -U flash:w:main.hex
