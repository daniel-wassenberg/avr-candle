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
#define TIMER0_OVF_RATE     (F_CPU / (64 * 256)) // Hz
static void init_pwm() {
    // COM0A    = 00  (disabled)
    // COM0B    = 00  (disabled)
    // WGM0     = 000 (Normal mode)
    // CS       = 011 (1:64 with IO clock: ~586 Hz PWM if clock is 9.6Mhz)
    TCCR0A  = 0x00;         // 0000 0000
    TCCR0B  = 0x03;         // 0000 0011
    
    DDRB = (1 << DDB0) | (1 << DDB3) | (1 << DDB4);
}

static volatile bool tick = false;

// stack usage: 5 bytes
ISR(TIM0_OVF_vect) {
    static uint8_t cycles = 0;
    if (!cycles--) {
        tick = true;
        cycles = TIMER0_OVF_RATE / UPDATE_RATE;
    }
    
    OCR0A = TCNT0 + 1;
}

uint8_t pwm[3] = {};
const uint8_t pwm_pin[3] = {1<<PB0, 1<<PB3, 1<<PB4};

ISR(TIM0_COMPA_vect) {
    uint8_t now, next;
    do {
        now = TCNT0;
        next = 255;
        
        uint8_t port = pwm_pin[0] | pwm_pin[1] | pwm_pin[2];
        
        for (int i = 0; i < 3; i++) {
            if (now >= pwm[i]) {
                port &= ~pwm_pin[i];
            } else if (!next || pwm[i] < next) {
                next = pwm[i];
            }
        }
        
        PORTB = port;
    } while (now >= next);
    
    if (next) OCR0A = next;
}

int main(void)
{
    init_rand();
    init_pwm();
    
    // enable timer overflow interrupt
    TIMSK0 = (1 << TOIE0) | (1 << OCIE0A);
    
    static filter_state candle[3] = {};
    
    while(1)
    {
        cli();
        for (int i = 0; i < 3; i++) {
            pwm[i] = next_intensity(&candle[i]);
        }
        sei();
        
        // sleep till next update
        while (!tick) sleep_mode();
        tick = false;
    }
}