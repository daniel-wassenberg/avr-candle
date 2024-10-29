// Candle-flicker LED program for ATmega4809 based on analysis at[0]
// and IIR filter proposed and tested at [1].
// 
// Hardware/peripheral usage notes:
// LED control outputs on pin 20, 21 and 22.
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

#define F_CPU 1600000UL

static uint8_t next_intensity(filter_state *state) {
    const uint8_t m = 171, s = 2;
    const int16_t scale = (s * FILTER_STDDEV) / (255 - m);
    
    int16_t x = m + flicker_filter(state, normal()) / scale;
    return x < 0 ? 0 : x > 255 ? 255 : x;
}

// set up PWM on pin 20, 21, 22 (PORTD pin 0, 1, 2) using Timer / Counter 0 (TCA0)
#define TCA0_OVF_RATE    (F_CPU / (2 * 2 * 256)) // Hz
static void init_pwm() {
    TCA0.SINGLE.CTRLA   = TCA_SINGLE_CLKSEL_DIV2_gc      // clock divide by 2
                        | TCA_SINGLE_ENABLE_bm;          // TCA0 enabled
    
    TCA0.SINGLE.CTRLB   = TCA_SINGLE_WGMODE_DSBOTTOM_gc  // dual slope PWM mode
                        | TCA_SINGLE_CMP0EN_bm           // compare 0 enabled
                        | TCA_SINGLE_CMP1EN_bm           // compare 1 enabled
                        | TCA_SINGLE_CMP2EN_bm;          // compare 2 enabled
    
    TCA0.SINGLE.PERBUF  = 0xFF;                          // counter top of 255
    
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;             // overflow interrupt enabled
    
    PORTD.DIR           = PIN0_bm                        // port D, pin 0 is output
                        | PIN1_bm                        // port D, pin 1 is output
                        | PIN2_bm;                       // port D, pin 2 is output
    
    PORTMUX.TCAROUTEA   = PORTMUX_TCA0_PORTD_gc;         // move TCA0 to port D
}

static volatile bool tick = false;

// stack usage: 5 bytes
ISR(TCA0_OVF_vect) {
    static uint8_t cycles = 0;
    
    if (!cycles--) {
        tick = true;
        cycles = TCA0_OVF_RATE / UPDATE_RATE;
    }
    
    // clear interrupt flag
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// pointers to timer 0 (TCA0) compare registers
volatile uint16_t* pwm[3] = {&TCA0.SINGLE.CMP0BUF,
                             &TCA0.SINGLE.CMP1BUF,
                             &TCA0.SINGLE.CMP2BUF};

int main(void)
{
    static filter_state candle[3] = {};
    
    CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_10X_gc     // main clock divide by 10
                      | CLKCTRL_PEN_bp;         // enable clock divider
    
    PORTE.DIR         = PIN2_bm;                // port E, pin 2 is output
            
    init_rand();
    
    init_pwm();
    
    sei();
    
    while(1)
    {
        for (int i = 0; i < 3; i++) {
            *pwm[i] = next_intensity(&candle[i]);
        }
        
        // toggle led (pin 32) on Arduino Nano
        PORTE.OUTTGL  = PIN2_bm;
        
        // sleep till next update
        while (!tick) sleep_mode();
        
        tick = false;
    }
}