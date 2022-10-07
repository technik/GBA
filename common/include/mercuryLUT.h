#pragma once

#include "linearMath.h"

// LUT table that returns the sin(x) as sNorm_16 (i.e. 1 bit for sign, 1 bit integer part, 14 bits precision.
// x maps the range of [0,2*pi) to the range[0,0x1ff].
extern const int16_t SinP9LUT[];
// LUT table that returns the cotan(x)=cos(x)/sin(x) as sNorm_16 (i.e. 1 bit for sign, 1 bit integer part, 14 bits precision.
// Only valid for cotan(x)<=1.
// x maps the range of [pi/4,3*pi/4) to the range[0,0x1ff].
extern const int16_t CotanP9LUT[];
