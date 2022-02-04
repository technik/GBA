#include <cstdint>
#include "linearMath.h"
#include "Display.h"
#include "Draw.h"
#include "Timer.h"
#include <cmath>

using namespace math;

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

void trace(uint16_t* backBuffer, int32_t t)
{
	constexpr uint32_t bgBlue = (0x1f<<10)|(0x1f<<5)|(160>>3);

	Vec2p8 uv;
	constexpr auto invY = 2_p8/ScreenHeight;
	constexpr auto invX = 2_p8/ScreenWidth;

	for(int32_t y = 0; y < ScreenHeight; ++y)
	{
		uv.y() = (intp8(y)*invY-1_p8)*3;
		auto y2_dst = uv.y()*uv.y()-0.5_p8;

		for(int32_t x = 0; x < ScreenWidth; ++x)
		{
			int32_t pixel = x+y*ScreenWidth;
			uv.x() = (intp8(x)*invX-1_p8)*4;

			auto dst = y2_dst+(uv.x()*uv.x());
			if(dst.raw < 0)
			{
				backBuffer[pixel] = 0xefff;
			}
			else
			{
				backBuffer[pixel] = bgBlue;
			}
		}
	}
}

void drawScene(uint16_t* backBuffer, int32_t t)
{
	constexpr uint32_t bgBlue = (0x1f<<10)|(0x1f<<5)|(160>>3);
	// Draw
	clear(backBuffer, bgBlue);
	constexpr auto halfSide = 20_p8;
	constexpr auto center = 100_p8;
	auto sx = intp8((float)sin(t*0.125f));
	auto ndl = max(0_p8, intp8((float)sin(t*0.125f+0.2f)));

	math::Vec2p8 tri[3] = {
		math::Vec2p8(center-halfSide*sx,100_p8),
		math::Vec2p8(center+halfSide*sx,100_p8),
		math::Vec2p8(center,40_p8)
	};

	rasterTriangle(backBuffer, (0x1f*(1+ndl)/2).round()<<5, tri);
}

int main()
{
	Display().Init();

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to ~1/16th of a millisecond
	while(1)
	{
		auto* displayBuffer = Display().backBuffer();
		// Logic
		//uint16_t backBuffer[ScreenWidth*ScreenHeight];
		drawScene(displayBuffer, t);
		//trace(displayBuffer, t);

		// VSync
		plotFrameIndicator(displayBuffer);
		Display().vSync();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
		Display().flipFrame();

		++t;
	}
	return 0;
}