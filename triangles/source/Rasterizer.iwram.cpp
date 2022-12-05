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
	intp16 x = abs(p.m_x);
	intp16 y = abs(p.m_y);
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