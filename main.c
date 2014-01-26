// Candle-flicker LED program for ATtiny13A based on analysis at[0]
// and IIR filter proposed and tested at [1].
// 
// Hardware/peripheral usage notes:
// LED control output on pin 5.
// Uses hardware PWM and the PWM timer's overflow to
// count out the frames.  Keeps the RNG seed in EEPROM.
// 
// [0] http://inkofpark.wordpress.com/2013/12/15/candle-flame-flicker/
// [1] http://inkofpark.wordpress.com/2013/12/23/arduino-flickering-candle/

#include "filter.h"
#include "rand.h"

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdbool.h>

static uint8_t next_intensity(filter_state *state) {
    const uint8_t m = 171, s = 2;
    const int16_t scale = (s * FILTER_STDDEV) / (255 - m);
    
    int16_t x = m + flicker_filter(state, normal()) / scale;
    return x < 0 ? 0 : x > 255 ? 255 : x;
}

// set up PWM on pin 5 (PORTB bit 0) using TIMER0 (OC0A)
#define TIMER0_OVF_RATE     2343.75 // Hz
static void init_pwm() {
    // COM0A    = 10  ("normal view")
    // COM0B    = 00  (disabled)
    // WGM0     = 001 (phase-correct PWM, TOP = 0xff, OCR update at TOP)
    // CS       = 010 (1:8 with IO clock: 2342 Hz PWM if clock is 9.6Mhz )
    TCCR0A  = 0x81;         // 1000 0001
    TCCR0B  = 0x02;         // 0000 0010
    DDRB   |= 1 << DDB0;    // pin direction = OUT
}

static volatile bool tick = false;

// stack usage: 5 bytes
ISR(TIM0_OVF_vect) {
    static uint8_t cycles = 0;
    if (!cycles--) {
        tick = true;
        cycles = TIMER0_OVF_RATE / UPDATE_RATE;
    }
}

int main(void)
{
    init_rand();
    init_pwm();
    
    // enable timer overflow interrupt
    TIMSK0 = 1 << TOIE0;
    sei();
    
    static filter_state candle = {};
    
    while(1)
    {
        OCR0A   = next_intensity(&candle);
        
        // sleep till next update
        while (!tick) sleep_mode();
        tick = false;
    }
}