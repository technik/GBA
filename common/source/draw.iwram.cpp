#include "Draw.h"

using namespace math;

void rasterTriangle(uint16_t* dst, math::Vec2i scissor, uint16_t color, const math::Vec2p8 v[3])
{
	// Back face culling
	auto e0 = v[1] - v[0];
	auto e1 = v[2] - v[1];
	auto e2 = v[0] - v[2];
	math::Vec2p8 p;

	if (cross(e0, e1) >= 0)
		return;

	// Locate boundaries
	auto x0 = v[0].x;
	auto x1 = v[1].x;
	auto x2 = v[2].x;
	auto xStart = math::max(0_p8, math::min(x0, math::min(x1, x2)) - 0.5_p8).floor();
	auto xEnd = math::min(intp8(scissor.x), math::max(x0, math::max(x1, x2)) - 0.5_p8).floor() + 1;

	auto y0 = v[0].y;
	auto y1 = v[1].y;
	auto y2 = v[2].y;

	auto yStart = math::max(0_p8, math::min(y0, math::min(y1, y2)) - 0.5_p8).floor();
	auto yEnd = math::min(intp8(scissor.y), math::max(y0, math::max(y1, y2)) - 0.5_p8).floor() + 1;

	// Parse bounding rectangle looking for intersections
	for (auto y = yStart; y < yEnd; y += 1)
	{
		p.y = intp8(y);
		for (auto x = xStart; x < xEnd; x += 1)
		{
			p.x = intp8(x);

			if (cross(p - v[0], e0) > 0)
				if (cross(p - v[1], e1) > 0)
					if (cross(p - v[2], e2) > 0)
						dst[x + y * scissor.x] = color;
		}
	}
}