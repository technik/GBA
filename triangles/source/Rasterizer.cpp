//
// Sector rasterizer code that doesn't need to fit in IWRAM
//

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <Draw.h>
#include <linearMath.h>

#include <gfx/palette.h>
#include <Rasterizer.h>

// Maps
#include <test.wad.h>
#include <mercury.wad.h>
#include <portaltest.wad.h>
#include <e1m1.wad.h>

using namespace math;
using namespace gfx;

// No need to place this method in fast memory
void Rasterizer::Init()
{
	displayMode.Init();
	Display().enableSprites();
}

bool Rasterizer::BeginFrame()
{
    return displayMode.BeginFrame();
}

void Rasterizer::EndFrame()
{
    displayMode.Flip();
}

void Rasterizer::DrawHorizontalLine(uint16_t* buffer, int stride, int16_t color, int row, int xStart, int xEnd)
{
	int rowStart = stride * row;
	for (int col = xStart; col < xEnd; ++col)
		buffer[rowStart + col] = color;
}

void Rasterizer::DrawVerticalLine(uint16_t * buffer, int stride, int16_t color, int col, int yStart, int yEnd)
{
	int rowStart = stride * yStart;
	for (int row = yStart; row < yEnd; ++row)
	{
		buffer[rowStart + col] = color;
		rowStart += stride;
	}
}