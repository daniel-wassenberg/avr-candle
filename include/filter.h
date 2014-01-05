#ifndef ___n_filter_h__
#define ___n_filter_h__

#include <stdint.h>

#define UPDATE_RATE     156.25 // Hz
#define FILTER_STDDEV   6.2e3

int16_t flicker_filter(int8_t x);

#endif /* ___n_filter_h__ */
