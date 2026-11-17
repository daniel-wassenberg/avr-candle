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
This was compiled using PlatformIO on VSCode and programed onto an Arduino Nano Every using the Arduino bootloader.

[0]: http://inkofpark.wordpress.com/2013/12/15/candle-flame-flicker
[1]: http://inkofpark.wordpress.com/2013/12/23/arduino-flickering-candle
[2]: https://github.com/mokus0