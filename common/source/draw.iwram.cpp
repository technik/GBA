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

	auto rowPtr = &dst[yStart * scissor.x];
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
				rowPtr[x] = color;
			}

			cx0 += e0.y;
			cx1 += e1.y;
			cx2 += e2.y;
		}

		rowPtr += scissor.x;

		cy0 -= e0.x;
		cy1 -= e1.x;
		cy2 -= e2.x;
	}
}

// Scanline raster of a solid color triangle
void rasterTriangleExp(uint16_t* dst, math::Vec2i scissor, uint16_t color, const math::Vec2p8 v[3])
{
	// Back face culling
	const Vec2p8 edge[3] = {
		v[1] - v[0],
		v[2] - v[1],
		v[0] - v[2]
	};

	if (cross(edge[0], edge[1]) >= 0)
		return;

	// Classify edges as left or right, and scan through them
	int8_t leftEdge[160];
	int8_t rightEdge[160];

	int yStart = 128;
	int yEnd = -1;
	int xMin = min(160, min3(v[0].x, v[1].x, v[2].x).floor());
	int xMax = max(-1, max3(v[0].x, v[1].x, v[2].x).floor()+1);

	for (int i = 0; i < 3; ++i)
	{
		if (edge[i].y > 0) { // Downward edge, left edge

			// Find the first row at or below the top vertex
			const auto v0y = v[i].y;
			int y0 = (v0y).floor() + (((v0y).fract() <= 0.5_p8) ? 0 : 1);
			yStart = min(yStart, y0); // Adjust the triangle boundaries to contain this edge

			// Find the first row guaranteed to be past the bottom vertex
			const auto v1y = v0y + edge[i].y;
			int y1 = (v1y).floor() + (((v1y).fract() <= 0.5_p8) ? 0 : 1);
			yEnd = max(yEnd, y1); // Adjust the triangle boundaries to contain this edge

			int x0 = (v[i].x).floor() + (((v[i].x).fract() <= 0.5_p8) ? 0 : 1);
			// Scan the edge
			if(edge[i].x > 0) // Edge leaning right
			{
				auto rOff = ((x0 + 0.5_p8 - v[i].x) * edge[i].y - edge[i].x * (y0 + 0.5_p8 - v0y)).round<8>();
				for (int row = y0; row < y1; ++row)
				{
					while (rOff < 0)
					{
						x0++;
						rOff += edge[i].y;
					}
					// while
					if (rOff >= 0)
					{
						leftEdge[row] = x0;
						dst[x0 + row * scissor.x] = color;
					}
					rOff -= edge[i].x;
				}
			}
			else if(edge[i].x < 0)
			{ }
			else // Vertical edge
			{ 
				for(int row = y0; row < y1; ++row)
				{
					leftEdge[row] = x0;
					dst[x0 + row * scissor.x] = color;
				}
			}
		} else if(edge[i].y < 0) { // Upward edge, right edge
			//
		}else { // Horizontal edge
			//
		}
	}

	// Locate boundaries
	auto x0 = v[0].x;
	auto x1 = v[1].x;
	auto x2 = v[2].x;
	auto xStart = math::max(0_p8, math::min3(x0, x1, x2) - 0.5_p8).floor();
	auto xEnd = math::min(intp8(scissor.x), math::max3(x0, x1, x2) - 0.5_p8).floor();

	auto y0 = v[0].y;
	auto y1 = v[1].y;
	auto y2 = v[2].y;
	//auto yStart = math::max(0_p8, math::min3(y0, y1, y2) - 0.5_p8).floor();
	//auto yEnd = math::min(intp8(scissor.y), math::max3(y0, y1, y2) - 0.5_p8).floor();

	// Parse bounding rectangle looking for intersections
	math::Vec2p8 p = { intp8(xStart) + 0.5_p8, intp8(yStart) + 0.5_p8 };

	// Cross products of the first pixel center within the bounding box
	auto cy0 = cross(p - v[0], edge[0]).cast<8>();
	auto cy1 = cross(p - v[1], edge[1]).cast<8>();
	auto cy2 = cross(p - v[2], edge[2]).cast<8>();

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
				//dst[x + y * scissor.x] = color;
			}

			cx0 += edge[0].y;
			cx1 += edge[1].y;
			cx2 += edge[2].y;
		}

		cy0 -= edge[0].x;
		cy1 -= edge[1].x;
		cy2 -= edge[2].x;
	}
}