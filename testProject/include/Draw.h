#pragma once

#include <cstdint>
#include "Display.h"

inline void bmp16_line(int x1, int y1, int x2, int y2, uint16_t clr,
    uint16_t *dstBase, uint16_t dstPitch)
{
    int ii, dx, dy, xstep, ystep, dd;
    uint16_t *dst= (uint16_t*)(dstBase + y1*dstPitch + x1*2);
    dstPitch /= 2;

    // --- Normalization ---
    if(x1>x2)
    {
		xstep = -1;
		dx = x1-x2;
	}
    else
    {
		xstep = 1;
		dx = x2-x1;
	}

    if(y1>y2)
    {
		ystep = -dstPitch;
		dy= y1-y2;
	}
    else
    {
		ystep= +dstPitch;
		dy= y2-y1;
	}

    // --- Drawing ---

    if(dy == 0)         // Horizontal
    {
        for(ii=0; ii<=dx; ii++)
            dst[ii*xstep]= clr;
    }
    else if(dx == 0)    // Vertical
    {
        for(ii=0; ii<=dy; ii++)
            dst[ii*ystep]= clr;
    }
    else if(dx>=dy)     // Diagonal, slope <= 1
    {
        dd= 2*dy - dx;

        for(ii=0; ii<=dx; ii++)
        {
            *dst= clr;
            if(dd >= 0)
            {   dd -= 2*dx; dst += ystep;  }

            dd += 2*dy;
            dst += xstep;
        }               
    }
    else                // Diagonal, slope > 1
    {
        dd= 2*dx - dy;

        for(ii=0; ii<=dy; ii++)
        {
            *dst= clr;
            if(dd >= 0)
            {   dd -= 2*dy; dst += xstep;  }

            dd += 2*dx;
            dst += ystep;
        }       
    }
}

inline void clear(uint16_t* dst, uint16_t color)
{
	//uint32_t color2 = color<<16 | color;
	for(uint8_t y = 0; y < ScreenHeight; ++y)
	{
		for(uint8_t x = 0; x < ScreenWidth; ++x)
		{
			dst[x+y*ScreenWidth] = color;
		}
	}
}