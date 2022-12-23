#include "Draw.h"

using namespace math;

void Gradient(uint16_t* dst)
{
	for(uint8_t y = 0; y < ScreenHeight; ++y)
	{
		for(uint8_t x = 0; x < ScreenWidth; ++x)
		{
			dst[x+y*ScreenWidth] = ((y>>3)<<5) | (x>>3);
		}
	}
}