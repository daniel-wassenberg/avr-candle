// scipy-designed filter with 4Hz cutoff and 100Hz sample rate.
// parameters set using the following little snippet:
// >>> rate=156.25
// >>> cutoff = 2.2/(rate/2.)
// >>> b, a = signal.butter(2, cutoff, 'low')
//
// which yields:
//
//  b = 0.00184036,  0.00368073,  0.00184036
//  a = 1.        , -1.87503716,  0.88239861
//
// B was then rescaled to 1, 2, 1 in order to improve the
// numerical accuracy by both reducing opportunities for 
// round-off error and increasing the amount of precision
// that can be allocated in other places.  (it also
// conveniently replaces 3 multiplies with shifts)
//
// This function implements the transposed direct-form 2, using
// 16-bit fixed-point math.  Integer types here are annotated
// with comments of the form "/* x:y */", where x is the number
// of significant bits in the value and y is the (negated)
// base-2 exponent.
// 
// The inspiration for the filter (and identification of basic
// parameters and comparison with some other filters) was done
// by Park Hays and described at [0].  Some variations and
// prototypes of the fixed-point version are implemented at [1].
// The specific parameters of this filter, though, are changed
// as follows:
// 
// The higher sampling rate puts the nyquist frequency at 50 Hz
// (digital butterworth filters appear to roll off  to -inf at
// the nyquist rate, though the fixed-point implementation has
// a hard noise floor due to the fixed quantum).  This probably
// doesn't make that much difference visually but it seems to
// greatly improve the numerical properties of the fixed-point
// version.
// 
// SciPy's butterworth parameter designer also seems to put the
// cutoff frequency higher than advertised (or maybe that's an
// effect of the limited sampling rate, but PSD plots seem to
// show the cutoff much higher than expected, and an 8Hz cutoff
// was just far too fast-moving for my taste), so I pulled it
// down to something that looked pleasant when running, yielding
// the following filter.
// 
// stack usage: 10 bytes (plus whatever __mulsi3 uses - probably 2 bytes)
// 
// [0] http://inkofpark.wordpress.com/2013/12/23/arduino-flickering-candle/
// [1] https://github.com/mokus0/junkbox/blob/master/Haskell/Math/BiQuad.hs

#include "filter.h"

int16_t /* 15:13 */ flicker_filter(int8_t /* 7:5 */ x) {
    const int16_t
        /* 9:8 */ a1 = -480, // round ((-1.87503716) * (1L << 8 ))
        /* 8:8 */ a2 =  226; // round (  0.88239861  * (1L << 8 ))
        // b0 = 1, b1 = 2, b2 = 1; multiplies as shifts below
    static int16_t
        /* 15:6 */ d1 = 0,
        /* 15:6 */ d2 = 0;
    
    int16_t /* 15:6 */ y;
    
    y  = ((int16_t) x << 1)                           + (d1 >> 0);
    d1 = ((int16_t) x << 2) - (a1 * (int32_t) y >> 8) + (d2 >> 0);
    d2 = ((int16_t) x << 1) - (a2 * (int32_t) y >> 8);
    
    return y;
}
