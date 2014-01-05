#include "rand.h"

#include <avr/eeprom.h>

// simple 32-bit LFSR PRNG with seed stored in EEPROM
#define POLY 0xA3AC183Cul
static uint32_t lfsr = 1;

// Seed the PRNG from the seed saved in EEPROM
// stack usage: 2 bytes
void init_rand() {
    static uint32_t EEMEM boot_seed = 1;
    
    lfsr = eeprom_read_dword(&boot_seed);
    // increment at least once, skip 0 if we hit it
    // (0 is the only truly unacceptable seed)
    do {lfsr++;} while (!lfsr);
    eeprom_write_dword(&boot_seed, lfsr);
}

// stack usage: 2 bytes
uint8_t rand(uint8_t bits) {
    uint8_t x = 0;
    uint8_t i;
    
    for (i = 0; i < bits; i++) {
        x <<= 1;
        lfsr = (lfsr >> 1) ^ (-(lfsr & 1ul) & POLY);
        x |= lfsr & 1;
    }
    
    return x;
}

// approximate a normal distribution with mean 0 and std 32.
// does so by drawing from a binomial distribution and 'fuzzing' it a bit.
// 
// There's some code at [0] calculating the actual distribution of these
// values.  They fit the intended distribution quite well.  There is a
// grand total of 3.11% of the probability misallocated, and no more than
// 1.6% of the total misallocation affects any single bin.  In absolute
// terms, no bin is more than 0.05% off the intended priority, and most
// are considerably closer.
// 
// stack usage: (2 + 2) = 4 bytes 
// 
// [0] https://github.com/mokus0/junkbox/blob/master/Haskell/Math/ApproxNormal.hs
int8_t normal() {
    // n = binomial(16, 0.5): range = 0..15, mean = 8, sd = 2
    // center = (n - 8) * 16; // shift and expand to range = -128 .. 112, mean = 0, sd = 32
    int8_t center = -128;
    uint8_t i;
    for (i = 0; i < 16; i++) {
        center += rand(1) << 4;
    }
    
    // 'fuzz' follows a symmetric triangular distribution with 
    // center 0 and halfwidth 16, so the result (center + fuzz)
    // is a linear interpolation of the binomial PDF, mod 256.
    // (integer overflow corresponds to wrapping around, blending
    // both tails together).
    int8_t fuzz = (int8_t)(rand(4)) - (int8_t)(rand(4));
    return center + fuzz;
}
