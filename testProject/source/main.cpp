#include <cstdint>
#include "Display.h"
#include "Draw.h"
#include "Timer.h"

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
	Display().set<VideoMode, DisplayControl::BG2>();

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
		Display().vSync();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
		Display().flipFrame();

		// Copy display
		auto* displayBuffer = Display().backBuffer();
		for(int i = 0; i < ScreenHeight*ScreenWidth; ++i)
		{
			displayBuffer[i] = backBuffer[i];
		}

		++t;
	}
	return 0;
}