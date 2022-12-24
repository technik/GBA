#include "Draw.h"

using namespace math;

// Scanline raster of a solid color triangle
void rasterTriangle(uint16_t* dst, math::Vec2i scissor, uint16_t color, const math::Vec2p8 v[3])
{
	// Back face culling
	auto e0 = v[1] - v[0];
	auto e1 = v[2] - v[1];
	auto e2 = v[0] - v[2];

	if (cross(e0, e1) >= 0)
		return;

	// Locate boundaries
	auto x0 = v[0].x;
	auto x1 = v[1].x;
	auto x2 = v[2].x;
	auto xStart = math::max(0_p8, math::min3(x0, x1, x2)-0.5_p8).floor();
	auto xEnd = math::min(intp8(scissor.x), math::max3(x0, x1, x2) - 0.5_p8).floor();

	auto y0 = v[0].y;
	auto y1 = v[1].y;
	auto y2 = v[2].y;
	auto yStart = math::max(0_p8, math::min3(y0, y1, y2) - 0.5_p8).floor();
	auto yEnd = math::min(intp8(scissor.y), math::max3(y0, y1, y2) - 0.5_p8).floor();

	// Parse bounding rectangle looking for intersections
	math::Vec2p8 p = { intp8(xStart) + 0.5_p8, intp8(yStart) + 0.5_p8};

	// Cross products of the first pixel center within the bounding box
	auto cy0 = cross(p - v[0], e0).cast<8>();
	auto cy1 = cross(p - v[1], e1).cast<8>();
	auto cy2 = cross(p - v[2], e2).cast<8>();

	for (auto y = yStart; y < yEnd; y += 1)
	{
		// Reset cross products at the start of the line
		auto cx0 = cy0;
		auto cx1 = cy1;
		auto cx2 = cy2;

		p.y = intp8(y) + 0.5_p8;
		for (auto x = xStart; x < xEnd; x += 1)
		{
			p.x = intp8(x) + 0.5_p8;

			if ((cx0 > 0) && (cx1 > 0) && (cx2 > 0))
			{
				dst[x + y * scissor.x] = color;
			}

			cx0 += e0.y;
			cx1 += e1.y;
			cx2 += e2.y;
		}

		cy0 -= e0.x;
		cy1 -= e1.x;
		cy2 -= e2.x;
	}
}