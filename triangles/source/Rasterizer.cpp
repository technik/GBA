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
	//clear(Display().backBuffer(), skyClr.raw, groundClr.raw, displayMode.Area);

	const Vec3p16 vertices[3] = {
		{-0.5_p16, -0.5_p16, -2_p16},
		{ 0.5_p16, -0.5_p16, -2_p16},
		{ 0_p16,    0.5_p16, -2_p16},
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

// Follow Bresenham's algorithm for line rasterization
void Rasterizer::DrawLine(uint16_t* buffer, int stride, int16_t color, math::Vec2p16 a, math::Vec2p16 b, int xEnd, int yEnd)
{
	if (b.x() < a.x())
	{
		DrawLine(buffer, stride, color, b, a, xEnd, yEnd);
		return;
	}

	// Compute deltas
	auto dx = b.x() - a.x();
	auto dy = b.y() - a.y();

	if (dx == 0)
	{
		if (dy > 0)
			DrawVerticalLine(buffer, stride, color, a.x().floor(), a.y().floor(), b.y().floor() + 1);
		else
			DrawVerticalLine(buffer, stride, color, a.x().floor(), b.y().floor(), a.y().floor() + 1);
		return;
	}
	if (dy == 0)
	{
		DrawHorizontalLine(buffer, stride, color, a.y().floor(), a.x().floor(), b.x().floor() + 1);
		return;
	}

	intp16 m = dy / dx; // Slope

	// Clamp to the screen
	if (a.x() < 0)
	{
		a.y() -= a.x() * m;
		a.x() = 0_p16;
	}

	while (a.y() < 0)
	{
		a.x() += 1_p16;
		a.y() += m;
	}

	// Where to start drawing
	int col = a.x().floor();
	int row = a.y().floor();
	int rowStart = stride * a.y().floor();
	int endCol = math::min<int>(xEnd, b.x().floor());
	int endRow = math::min<int>(yEnd, b.y().floor());

	// Shift pixel centers to integer coordinates
	a.x() -= 0.5_p16;
	a.y() -= 0.5_p16;
	b.x() -= 0.5_p16;
	b.y() -= 0.5_p16;

	if (dy <= 0) // First quadrant
	{
		if (dx <= -dy) // 2nd octant
		{
			a.x() = a.x().fract();
			m = dx / dy;
			while (row >= endRow && col < xEnd)
			{
				buffer[col + rowStart] = color;
				a.x() += m;
				rowStart -= stride;
				--row;
				if (a.x() < 0)
				{
					a.x() += 1;
					col++;
				}
			}
		}
		else // 1st octant
		{
			a.y() = a.y().fract();
			while (col <= endCol && row >= 0)
			{
				buffer[col + rowStart] = color;
				a.y() += m;
				++col;
				if (a.y() < 0)
				{
					a.y() += 1;
					row--;
					rowStart -= stride;
				}
			}
		}
	}
	else // 4th quadrant
	{
		if (dx <= dy) // 7th octant
		{
			a.x() = a.x().fract();
			m = dx / dy;
			while(row <= endRow && col < xEnd)
			{
				buffer[col + rowStart] = color;
				a.x() += m;
				rowStart += stride;
				++row;
				if (a.x() >= 1)
				{
					a.x() -= 1;
					col++;
				}
			}
		}
		else // 8th octant
		{
			a.y() = a.y().fract();
			while(col <= endCol && row < yEnd)
			{
				buffer[col + rowStart] = color;
				a.y() += m;
				++col;
				if (a.y() >= 1)
				{
					a.y() -= 1;
					row++;
					rowStart += stride;
				}
			}
		}
	}

}