// Test the fixed point math library we use on the GBA
#include <linearMath.h>
#include <vector.h>
#include <Camera.h>
#include <cassert>

using namespace math;

// TODO: Optimize this with a LUT to avoid the BIOS call
intp16 fastAtan2(intp8 x, intp8 y)
{
	// Determines the final atan's sign
	int sign = sgn(x.raw * y.raw);
	// Map to the first quadrant
	int x1 = abs(x.raw);
	int y1 = y.raw;
	// Map angle to the [0,Pi/4] range.
	int atan16;
	if (x1 >= y1)
	{
		atan16 = ArcTan((x1 << 14) / y1);
	}
	else
	{
		atan16 = (1 << 12) - ArcTan((y1 << 8) / x1);
	}
	return (x.raw >= 0) ?
		intp16::castFromShiftedInteger<16>(atan16) :
		0.5_p16 - intp16::castFromShiftedInteger<16>(atan16);
}

void testLinearMath()
{
    intp12 depth = 1_p12;
    assert(depth.raw = 1 << 12);

    auto casted = (2 * depth / 160_p12).cast<8>();
    auto manual = (2 * depth.raw / 160 + (1 << 3)) / 16;

    assert(casted.raw == manual);

    intp12 dx = 0.01_p12;
    depth += dx;

    int16_t regA = (2 * depth / 160_p12).cast<8>().raw;
    depth.raw = -984;

    int32_t sin_phi = (-1) << 12;
    auto s1 = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(sin_phi);

    assert(s1.raw == sin_phi / 16);

    intp8 x;
    x.raw = 30650;
    int16_t ssx = x.roundToInt();

    assert(ssx == 120);
}

bool clipWall(Vec2p8& v0, Vec2p8& v1)
{

	constexpr intp8 nearClip = intp8(1 / 64.f);
	// Clip behind the view.
	if (v0.y() <= nearClip && v1.y() <= nearClip)
		return false;

	// Clip against the y=0 line
	// Skip walls fully positive or parallel to y=0
	if ((v0.y() < nearClip || v1.y() < nearClip) && !(v0.y() == v1.y()))
	{
		intp8 dX = v1.x() - v0.x();
		intp12 denom = (v1.y() - v0.y()).cast<12>();
		intp12 num = (dX * (nearClip - v0.y())).cast<12>();
		intp8 xClip = v0.x() + (num / denom).cast<8>();

		if (v0.y() < 0)
		{
			v0.x() = xClip;
			v0.y() = nearClip;
		}
		else if (v1.y() < 0)
		{
			v1.x() = xClip;
			v1.y() = nearClip;
		}
	}

	// Clip back facing walls
	if (v0.x() * v1.y() >= v1.x() * v0.y())
		return false;

	// Compute endpoint angles
	intp16 angle0 = fastAtan2(v0.x(), v0.y());
	intp16 angle1 = fastAtan2(v1.x(), v1.y());

	// Clip angles to the visible view frustum
	// Shifting by a quarter revolution gives us the angle to "y", the view direction
	constexpr intp16 clipAngle = 0.07379180882521663_p16; // atan(0.5) = fov/2
	constexpr intp16 leftClip = 0.25_p16 + clipAngle;
	constexpr intp16 rightClip = 0.25_p16 - clipAngle;
	if (angle1 > leftClip) // Past the left side of the screen
	{
		return false;
	}
	else
	{
		angle1 = max(rightClip, angle1);
	}
	if (angle0 < rightClip) // Past the right side of the screen
	{
		return false;
	}
	else
	{
		angle0 = min(leftClip, angle0);
	}

	if (angle0 <= angle1)
		return false;

	// Shift to improve numerical precision
	angle0.raw += 1 << 6;
	angle1.raw += 1 << 6;

	// Reconstruct clipped vertices
	auto cos0 = intp12::castFromShiftedInteger<12>(lu_cos(angle0.raw));
	auto sin0 = intp12::castFromShiftedInteger<12>(lu_sin(angle0.raw));
	auto cos1 = intp12::castFromShiftedInteger<12>(lu_cos(angle1.raw));
	auto sin1 = intp12::castFromShiftedInteger<12>(lu_sin(angle1.raw));

	intp8 dX = v1.x() - v0.x();
	intp8 dY = v1.y() - v0.y();
	intp8 num = (v0.x() * dY - v0.y() * dX).cast<8>();

	intp8 h0 = num / (cos0 * dY - sin0 * dX).cast<8>();
	intp8 h1 = num / (cos1 * dY - sin1 * dX).cast<8>();

	v0.x() = (cos0 * h0).cast<8>();
	v0.y() = (sin0 * h0).cast<8>();
	v1.x() = (cos1 * h1).cast<8>();
	v1.y() = (sin1 * h1).cast<8>();

	return true;
}

void testClip()
{
	Vec2p8 A;
	A.m_x.raw = -3684;
	A.m_y.raw = -591;
	Vec2p8 B;
	B.m_x.raw = 232;
	B.m_y.raw = 2705;
	assert(clipWall(A, B));

	// Fully in front
	A = Vec2p8(-1_p8, 1_p8);
	B = Vec2p8(1_p8, 1_p8);
	assert(clipWall(A, B));
	A = Vec2p8(-1_p8, 1_p8);
	B = Vec2p8(1_p8, 1_p8);
	assert(!clipWall(B, A)); // Backface

	// Fully behind
	A = Vec2p8(-1_p8, -1_p8);
	B = Vec2p8(1_p8, -1_p8);
	assert(!clipWall(A, B));
	A = Vec2p8(-1_p8, -1_p8);
	B = Vec2p8(1_p8, -1_p8);
	assert(!clipWall(B, A)); // Backface

	// Right side
	A = Vec2p8(1_p8, 1_p8);
	B = Vec2p8(1_p8, -1_p8);
	assert(clipWall(A, B));
	A = Vec2p8(1_p8, 1_p8);
	B = Vec2p8(1_p8, -1_p8);
	assert(!clipWall(B, A)); // Backface

	// Left side
	A = Vec2p8(-1_p8, -1_p8);
	B = Vec2p8(-1_p8, 1_p8);
	assert(clipWall(A, B));
	A = Vec2p8(-1_p8, -1_p8);
	B = Vec2p8(-1_p8, 1_p8);
	assert(!clipWall(B, A)); // Backface
}

int main()
{
	testClip();
    testLinearMath();

    return 0;
}