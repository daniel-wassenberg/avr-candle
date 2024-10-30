# avr-candle

## Overview
This program creates three PWM outputs that simulates the flicker of three candles on using ATmega4809 or Ardunio Nano Every. Each output is a randomly generated signal based on analysis [here][0] and IIR filter proposed and tested [here][1] with the orginal program written by [mokus0][2] for the ATtiny13A.

The PWM operates at a frequency of 1,562.5 Hz with the 16 MHz on chip clock. The duty cycle for each output is changed every 10 cycles for an update rate of 156.25 Hz.

## Pinouts

| Output     | ATmega Pin Name | Ardunio Pin Name  |
|:-----------|:---------------:|:-----------------:|
| PWM 1      | PD0             | D17               |
| PWM 2      | PD1             | D16               |
| PWM 3      | PD2             | D15               |
| Status LED | PE2             | D13               |

## Building
This was compiled using MPLAB X IDE and programed onto an Arduino Nano Every using the Arduino bootloader with help from Adam@SheekGeek's blog post on [Using MPlabX to Program Arduino Boards in Straight C/C++][3].

The blog post has you use the avr-gcc and avrdude that are included with the Arduino IDE, but I found that with the Nano Every you need to put it in a programming mode first and that required a newer version of avrdude that supports the `-r` flag. I ended up downloading avrdude version 8.0.0.0 and used the following template in MPLAB X to program the Arduino:

`/path/to/avrdude -C /path/to/avrdude.conf -r -v -V -p atmega4809 -c jtag2updi -P <<comm_port>> -b 115200 -e -D -U flash:w:${ImagePath}:i -U fuse2:w:0x01:m -U fuse5:w:0xC9:m -U fuse8:w:0x00:m {upload.extra_files}`

[0]: http://inkofpark.wordpress.com/2013/12/15/candle-flame-flicker
[1]: http://inkofpark.wordpress.com/2013/12/23/arduino-flickering-candle
[2]: https://github.com/mokus0
[3]: https://sheekgeek.org/2024/adamsheekgeek/using-mplabx-to-program-arduino-boards-in-straight-c-c