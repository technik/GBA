//
// mode7.iwram.c
// Interrupts
//
extern "C" {
	#include <tonc.h>
}
#include <Device.h>
#include <Color.h>
#include <gfx/palette.h>
#include <raycaster.h>

using namespace math;

// Actually draws two pixels at once
void verLine(uint16_t* backBuffer, unsigned x, unsigned drawStart, unsigned drawEnd, uint16_t worldColor)
{
	int16_t dPxl = worldColor | (worldColor<<8);
    constexpr unsigned stride = Mode4Display::Width/2;
	// Draw ceiling
	for(int i = 0; i < drawStart; ++i)
	{
		backBuffer[x + i*stride] = 1|(1<<8); // Sky color
	}
	// Draw wall
	for(int i = drawStart; i < drawEnd; ++i)
	{
		backBuffer[x + i*stride] = dPxl; // Wall color
	}
	// Draw ground
	for(int i = drawEnd; i < Mode4Display::Height; ++i)
	{
		backBuffer[x + i*stride] = 2|(2<<8); // Ground color
	}
}