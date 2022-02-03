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

int main()
{
	Display().Init();

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to ~1/16th of a millisecond
	const uint32_t bgBlue = (0x1f<<10)|(0x1f<<5)|(160>>3);
	while(1)
	{
		// Logic
		uint32_t height = t%ScreenHeight;

		uint16_t backBuffer[ScreenWidth*ScreenHeight];

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

		// VSync
		plotFrameIndicator(backBuffer);
		Display().vSync();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
		Display().flipFrame();

		// Copy display
		auto* displayBuffer = Display().backBuffer();
		for(uint32_t i = 0; i < ScreenHeight*ScreenWidth; ++i)
		{
			displayBuffer[i] = backBuffer[i];
		}

		++t;
	}
	return 0;
}