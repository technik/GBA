//
// Sector rasterizer code that doesn't need to fit in IWRAM
//

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
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

void clear(uint16_t* buffer, uint16_t topClr, uint16_t bottomClr, int area)
{
	DMA::Channel0().Fill(&buffer[0 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[1 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[2 * area / 4], bottomClr, area / 4);
	DMA::Channel0().Fill(&buffer[3 * area / 4], bottomClr, area / 4);
}

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

void Rasterizer::RenderWorld(const Camera& cam, const Mat44p16& projMtx)
{
	clear(Display().backBuffer(), skyClr.raw, groundClr.raw, displayMode.Area);

	const Vec3p16 vertices[3] = {
		{-0.5_p16, -0.5_p16, 0_p16},
		{ 0.5_p16, -0.5_p16, 0_p16},
		{ 0_p16,    0.5_p16, 0_p16},
	};

	Vec2p16 ssVertices[3];
	for (int i = 0; i < 3; ++i)
	{
		Vec3p16 csVtx = projectPosition(projMtx, vertices[i]);
		ssVertices[i] = { csVtx.x() * 80 + 80, csVtx.y() * 60 + 60 };
	}

	for (int i = 0; i < 3; ++i)
		DrawLine(
			Display().backBuffer(),
			displayMode.Width,
			BasicColor::Yellow.raw,
			ssVertices[i],
			ssVertices[(i+1)%3],
			displayMode.Width,
			displayMode.Height
		);

	/*
	Vec2p16 start = { 64_p16, 64_p16 };
	Vec2p16 end[16] = {
		{ 96_p16, 56_p16 },
		{ 72_p16, 32_p16 },
		{ 56_p16, 32_p16 },
		{ 32_p16, 56_p16 },
		{ 32_p16, 72_p16 },
		{ 56_p16, 96_p16 },
		{ 72_p16, 96_p16 },
		{ 96_p16, 72_p16 },
		{ 64_p16, 96_p16 },
		{ 96_p16, 64_p16 },
		{ 64_p16, 32_p16 },
		{ 32_p16, 64_p16 },
		{ 88_p16, 88_p16 },
		{ 40_p16, 88_p16 },
		{ 40_p16, 40_p16 },
		{ 88_p16, 40_p16 }
	};

	for(int i = 0; i < 16; ++i)
		DrawLine(
			Display().backBuffer(),
			displayMode.Width,
			BasicColor::Yellow.raw,
			start,
			end[i],
			displayMode.Width,
			displayMode.Height
		);
	*/
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