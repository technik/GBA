#pragma once

#include <linearMath.h>
#include <vector.h>

math::intp8 rayCast(math::Vec3p8 rayStart, math::Vec2p8 rayDir, int& hitVal, int& side, const uint8_t* map, int yStride);