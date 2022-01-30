#include <cstdint>
#include "Display.h"
#include "Timer.h"

// Hardware definition
#define REG_VCOUNT (*(volatile uint16_t*)(0x04000006))
#define REG_PALETTE (*(uint16_t*)(0x05000000))

// Config display
#define USE_VIDEO_MODE_5
#ifdef USE_VIDEO_MODE_3
	constexpr uint32_t ScreenWidth = 240;
	constexpr uint32_t ScreenHeight = 160;
	constexpr uint16_t VideoMode = 3;
#else // VIDEO MODE 5
	constexpr uint32_t ScreenWidth = 160;
	constexpr uint32_t ScreenHeight = 128;
	constexpr uint16_t VideoMode = 5;
#endif

void bmp16_line(int x1, int y1, int x2, int y2, uint16_t clr,
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

void clear(uint16_t* dst, uint16_t color)
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

void clear_videoMode5(uint16_t* dst, uint16_t color, uint16_t pitch, uint8_t x1, uint8_t y1)
{
	for(uint8_t y = 0; y < y1; ++y)
	{
		for(uint8_t x = 0; x < x1; ++x)
		{
			dst[x+y*pitch] = color;
		}
	}
}

void vsync()
{
	while(REG_VCOUNT > 160)
	{}
	while(REG_VCOUNT <= 160)
	{}
}

void plotFrameIndicator(uint16_t* frameBuffer)
{
	// Draw frame rate indicator
	uint16_t cnt = Timer0().counter;
	uint16_t frameCounterColor = 0xefff;
	if(cnt < 16*16) // >= 60fps
	{
		frameCounterColor = 0x1f<<5; // Green
	} else if(cnt < 33 * 16) // >=30 fps
	{
		frameCounterColor = 0x1f<<5 | 0x1f; // Yellow
	} else if(cnt < 50 * 16) // >=20 fps
	{
		frameCounterColor = 0x0f<<5 | 0x1f; // Orange
	}
	else if(cnt < 100 * 16) // >=10 fps
	{
		frameCounterColor = 0x1f; // Red
	}

	frameBuffer[0] = frameCounterColor;
}

int main()
{
	DisplayControl::Get().set<VideoMode, DisplayControl::BG2>();

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
	while(1)
	{
		// Logic
		const uint32_t blue = (t%0x1F)<<10;
		uint16_t height = t%ScreenHeight;

		uint16_t backBuffer[ScreenWidth*ScreenHeight];
		// Draw
		clear(backBuffer, blue);
		bmp16_line(0, height, ScreenWidth-1, height, 0xffff, backBuffer, ScreenWidth);

		// VSync
		plotFrameIndicator(backBuffer);
		vsync();
		DisplayControl::Get().flipFrame();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond

		// Copy display
		auto* displayBuffer = DisplayControl::Get().backBuffer();
		for(int i = 0; i < ScreenHeight*ScreenWidth; ++i)
		{
			displayBuffer[i] = backBuffer[i];
		}

		++t;
	}
	return 0;
}