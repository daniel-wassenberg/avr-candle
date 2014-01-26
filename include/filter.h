#ifndef ___n_filter_h__
#define ___n_filter_h__

#include <stdint.h>

#define UPDATE_RATE     156.25 // Hz
#define FILTER_STDDEV   6.2e3

typedef struct {
    int16_t
        /* 15:6 */ d1,
        /* 15:6 */ d2;
} filter_state;

int16_t flicker_filter(filter_state *state, int8_t x);

#endif /* ___n_filter_h__ */
