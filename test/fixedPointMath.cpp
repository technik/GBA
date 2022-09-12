// Test the fixed point math library we use on the GBA
#include <linearMath.h>
#include <vector.h>
#include <Camera.h>
#include <cassert>

using namespace math;

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
	// Clip behind the view. TODO: Support a non-zero near clip
	if (v0.y() <= 0 && v1.y() <= 0)
		return false;

	// Early out for no clipping
	if (v0.y() > 0 && v1.y() > 0)
		return true;

	if (v0.y() == v1.y()) // Nothing to clip here, the plane is parallel to the y=0 plane, so it can't intersect it.
		return true;

	intp8 dX = v1.x() - v0.x();
	intp8 yRef = v0.y();
	intp12 denom = (v1.y() - v0.y()).cast<12>();
	intp12 num = -(dX * yRef).cast<12>();

	if (v0.y() < 0)
	{
		v0.x() = v0.x() + (num / denom).cast<8>();
		v0.y() = 0_p8;
	}

	if (v1.y() < 0)
	{
		v1.x() = v1.x() + (num / denom).cast<8>();
		v1.y() = 0_p8;
	}

	return true;
}

void testClip()
{
	// Fully in front
	Vec2p8 A = Vec2p8(-1_p8, 1_p8);
	Vec2p8 B = Vec2p8(1_p8, 1_p8);
	assert(clipWall(A, B));

	// Fully behind
	A = Vec2p8(-1_p8, -1_p8);
	B = Vec2p8(1_p8, -1_p8);
	assert(!clipWall(A, B));

	// Right side
	A = Vec2p8(1_p8, 1_p8);
	B = Vec2p8(1_p8, -1_p8);
	assert(clipWall(A, B));

	// Left side
	A = Vec2p8(-1_p8, -1_p8);
	B = Vec2p8(-1_p8, 1_p8);
	assert(clipWall(A, B));
}

int main()
{
	testClip();
    testLinearMath();

    return 0;
}