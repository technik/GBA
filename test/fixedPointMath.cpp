// Test the fixed point math library we use on the GBA
#include <linearMath.h>
#include <cassert>

using namespace math;

int main()
{
    intp12 depth = 1_p12;
    assert(depth.raw = 1 << 12);

    auto casted = (2 * depth / 160_p12).cast_down<8>();
    auto manual = (2 * depth.raw / 160 + (1<<3)) / 16;

    assert(casted.raw == manual);

    intp12 dx = 0.01_p12;
    depth += dx;

    return 0;
}