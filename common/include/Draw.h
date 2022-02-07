#pragma once

#include <cstdint>
#include "Display.h"
#include "linearMath.h"
#include "vector.h"

void bmp16_line(int x1, int y1, int x2, int y2, uint16_t clr,
    uint16_t *dstBase, uint16_t dstPitch);

inline void clear(uint16_t* dst, uint16_t color)
{
	uint32_t color2 = color<<16 | color;
	uint32_t* buffer = reinterpret_cast<uint32_t*>(dst);
	for(uint32_t i = 0; i < (ScreenHeight*ScreenWidth/2); ++i)
	{
		buffer[i] = color2;
	}
}

inline void rect(uint16_t* dst, uint16_t color, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
	for(uint32_t y = y0; y < y1; ++y)
	{
		for(uint32_t x = x0; x < x1; ++x)
		{
			dst[x+y*ScreenWidth] = color;
		}
	}
}

void rasterTriangle(uint16_t* dst, uint16_t color, const math::Vec2p8 v[3]);

void Gradient(uint16_t* dst);