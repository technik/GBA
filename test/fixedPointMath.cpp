// Test the fixed point math library we use on the GBA
#include <linearMath.h>
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

void testCamera()
{
    auto camera = Camera(Vec3p8(256_p8, 256_p8, 1.7_p8));

    Vec3p8 objPos = camera.pos;
    objPos.y() += 5_p8;

    auto ss = camera.projectWorldPos(objPos);
    assert(ss.x().roundToInt() == ScreenWidth / 2);
    assert(ss.y().roundToInt() == ScreenHeight / 2);
    assert(ss.z().roundToInt() == 5);

    camera.cosf = intp8(cos(15.f / 180 * 3.1415927f));
    camera.sinf = intp8(sin(15.f / 180 * 3.1415927f));

    ss = camera.projectWorldPos(objPos);
    assert(ss.x().roundToInt() == 141);
    assert(ss.y().roundToInt() == ScreenHeight / 2);
    assert(ss.z().roundToInt() == 5);
}

int main()
{
    testLinearMath();
    testCamera();

    return 0;
}