//
// Performance critical code that need to be in IWRAM to keep 
// Sector rasterizer performance
//

#include <cstring>
#include <Camera.h>
#include <raycaster.h>

#include <container.h>
#include <Color.h>
#include <Device.h>
#include <linearMath.h>
#include <Rasterizer.h>

#include <gfx/palette.h>

#include <fastMath.h>

using namespace math;
using namespace gfx;

//#define FOV 90
//#define FOV 50
#define FOV 66

static constexpr uint32_t kMaxClipRanges = 32;

Color edgeClr[] = {
	BasicColor::Red,
	BasicColor::Orange,
	BasicColor::Yellow,
	BasicColor::Green,
	BasicColor::Blue,
	BasicColor::Pink,
	BasicColor::White,
	BasicColor::LightGrey,
	BasicColor::MidGrey,
	BasicColor::DarkGrey,
	BasicColor::DarkGreen
};

// TODO: Optimize this with a LUT to avoid the BIOS call
unorm16 fastAtan2(intp16 x, intp16 y)
{
	// Map angle to the first quadrant
	intp16 x1 = abs(x);
	intp16 y1 = abs(y);

	// Note: ArcTan takes a .14 fixed argument. To get the best precision, we perform the division shifts manually
	unorm16 atan16;
	if (x1 >= abs(y1)) // Lower half of Q1, upper half of Q4
	{
		// .16f << 6 = .22f
		// .14f = .22f / .8f
		int ratio = ((y.raw << 6) / (x1.raw>>8)); // Note we used y with sign to get both the tangent of the first and second quadrants correctly
		atan16 = unorm16::castFromShiftedInteger<16>(ArcTan(ratio) & 0xffff);
	}
	else // Upper half of Q1, lower half of Q4
	{
		// .14f = ( .16f << 6 ) / .8f
		int ratio = ((x1.raw << 6) / (y1.raw>>8));
		atan16 = 0.25_u16 - unorm16::castFromShiftedInteger<16>(ArcTan(ratio) & 0xffff);
		atan16 = y > 0 ? atan16 : (0_u16 - atan16);
	}
	return (x.raw >= 0) ? atan16 : 0.5_u16 - atan16;
}

intp16 PointToDist(const Vec2p16& p)
{
	// Rotate the point to the first octant.
	// We only care about distance, which is invariant to rotations
	intp16 x = abs(p.x);
	intp16 y = abs(p.y);
	if(y > x)
	{
		auto temp = x;
		x = y;
		y = temp;
	}

	auto ratio14 = (y / x).cast<14>(); // Note we used y with sign to get both the tangent of the first and second quadrants correctly
	intp16 atan16 = intp16::castFromShiftedInteger<16>(ArcTan(ratio14.raw));

	intp16 cosT = intp16::castFromShiftedInteger<12>(lu_cos(atan16.raw));
	return x / cosT;
}

// Follow Bresenham's algorithm for line rasterization
void Rasterizer::DrawLine(uint16_t* buffer, int stride, int16_t color, math::Vec2p16 a, math::Vec2p16 b, int xEnd, int yEnd)
{
	if (b.x < a.x)
	{
		DrawLine(buffer, stride, color, b, a, xEnd, yEnd);
		return;
	}

	// Compute deltas
	auto dx = b.x - a.x;
	auto dy = b.y - a.y;

	if (dx == 0)
	{
		if (dy > 0)
			DrawVerticalLine(buffer, stride, color, a.x.floor(), a.y.floor(), b.y.floor() + 1);
		else
			DrawVerticalLine(buffer, stride, color, a.x.floor(), b.y.floor(), a.y.floor() + 1);
		return;
	}
	if (dy == 0)
	{
		DrawHorizontalLine(buffer, stride, color, a.y.floor(), a.x.floor(), b.x.floor() + 1);
		return;
	}

	intp16 m = dy / dx; // Slope

	// Clamp to the screen
	if (a.x < 0)
	{
		a.y -= a.x * m;
		a.x = 0_p16;
	}

	while (a.y < 0)
	{
		a.x += 1_p16;
		a.y += m;
	}

	// Where to start drawing
	int col = a.x.floor();
	int row = a.y.floor();
	int rowStart = stride * a.y.floor();
	int endCol = math::min<int>(xEnd, b.x.floor());
	int endRow = math::min<int>(yEnd, b.y.floor());

	// Shift pixel centers to integer coordinates
	a.x -= 0.5_p16;
	a.y -= 0.5_p16;
	b.x -= 0.5_p16;
	b.y -= 0.5_p16;

	if (dy <= 0) // First quadrant
	{
		if (dx <= -dy) // 2nd octant
		{
			a.x = a.x.fract();
			m = dx / dy;
			while (row >= endRow && col < xEnd)
			{
				buffer[col + rowStart] = color;
				a.x += m;
				rowStart -= stride;
				--row;
				if (a.x < 0)
				{
					a.x += 1;
					col++;
				}
			}
		}
		else // 1st octant
		{
			a.y = a.y.fract();
			while (col <= endCol && row >= 0)
			{
				buffer[col + rowStart] = color;
				a.y += m;
				++col;
				if (a.y < 0)
				{
					a.y += 1;
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
			a.x = a.x.fract();
			m = dx / dy;
			while (row <= endRow && col < xEnd)
			{
				buffer[col + rowStart] = color;
				a.x += m;
				rowStart += stride;
				++row;
				if (a.x >= 1)
				{
					a.x -= 1;
					col++;
				}
			}
		}
		else // 8th octant
		{
			a.y = a.y.fract();
			while (col <= endCol && row < yEnd)
			{
				buffer[col + rowStart] = color;
				a.y += m;
				++col;
				if (a.y >= 1)
				{
					a.y -= 1;
					row++;
					rowStart += stride;
				}
			}
		}
	}

}