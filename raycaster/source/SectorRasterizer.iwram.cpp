//
// Performance critical code that need to be in IWRAM to keep 
// Sector rasterizer performance
//

#include <Camera.h>
#include <cstring>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>
#include <SectorRasterizer.h>

#include <gfx/palette.h>

using namespace math;
using namespace gfx;

//#define FOV 90
//#define FOV 50
#define FOV 66

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
intp16 fastAtan2(intp16 x, intp16 y)
{
	// Map angle to the first quadrant
	intp16 x1 = abs(x);
	intp16 y1 = abs(y);

	// Note: ArcTan takes a .14 fixed argument. To get the best precision, we perform the division shifts manually
	intp16 atan16;
	if (x1 >= abs(y1)) // Lower half of Q1, upper half of Q4
	{
		// .16f << 6 = .22f
		// .14f = .22f / .8f
		int ratio = ((y.raw << 6) / (x1.raw>>8)); // Note we used y with sign to get both the tangent of the first and second quadrants correctly
		atan16 = intp16::castFromShiftedInteger<16>(ArcTan(ratio));
	}
	else // Upper half of Q1, lower half of Q4
	{
		// .14f = ( .16f << 6 ) / .8f
		int ratio = ((x1.raw << 6) / (y1.raw>>8));
		atan16 = 0.25_p16 - intp16::castFromShiftedInteger<16>(ArcTan(ratio));
		atan16 = y > 0 ? atan16 : -atan16;
	}
	return (x.raw >= 0) ? atan16 : 0.5_p16 - atan16;
}

// Returns x in screen space, invDepth as y
Vec2p12 viewToClipSpace(const Vec2p16& viewSpace)
{
	dbgAssert(viewSpace.y() >= 0_p16);
	// Project x onto the screen
	intp12 invDepth = 1_p12 / max(intp12(1.f/64), viewSpace.y().cast<12>());
	// Assuming tg(fov_x/2) = 0.5
	Vec2p12 result;
	// Assuming tg(fov_x/2) = 1
#if FOV == 90
	result.x() = (viewSpace.x().cast<12>() * invDepth).cast<12>();
#elif FOV == 50
	result.x() = (viewSpace.x().cast<12>() * invDepth * 2).cast<12>();
#elif FOV == 66
	result.x() = (viewSpace.x().cast<12>() * invDepth * 3 / 2).cast<12>();
#endif
	result.y() = invDepth;
	return result;
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

	// .16f << 6 = .22f
		// .14f = .22f / .8f
	auto ratio14 = (y / x).cast<14>(); // Note we used y with sign to get both the tangent of the first and second quadrants correctly
	intp16 atan16 = intp16::castFromShiftedInteger<16>(ArcTan(ratio14.raw));

	intp16 cosT = intp16::castFromShiftedInteger<12>(lu_cos(atan16.raw));
	return x / cosT;
}

// Clips a wall that's already in view space.
// Returns whether the wall is visible.
bool clipWall(const Vec2p16& v0, const Vec2p16& v1, intp16 camAngle, Vec2p12& ndcA, Vec2p12& ndcB)
{
	// Compute endpoint angles
	intp16 angle0 = fastAtan2(v0.x(), v0.y());
	intp16 angle1 = fastAtan2(v1.x(), v1.y());

	// Clip back facing walls
	auto span = angle1 - angle0;
	span.raw &= 0xffff;
	dbgAssert(span >= 0_p16);
	if (span <= 0.5_p16)
	{
		return false;
	}

	intp16 rw_angle = angle0; // Keep this for computing invDepth
	angle0 -= camAngle;
	angle1 -= camAngle;

	constexpr intp16 nearClip = intp16(1/64.f);

	// Clip angles to the visible view frustum
	// Shifting by a quarter revolution gives us the angle to "y", the view direction
#if FOV == 90
	constexpr intp16 clipAngle = 0.125_p16; // atan(1.0) = fov/2. Corresponds to a fov of exactly 90 deg
#elif FOV == 50
	constexpr intp16 clipAngle = 0.07379180882521663_p16; // atan(0.5) = fov/2. Corresponds to a fov of about 50 deg
#elif FOV == 66
	constexpr intp16 clipAngle = 0.0935835209054994_p16; // atan(0.5) = fov/2. Corresponds to a fov of about 50 deg
#endif
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
	// This works because the internal LUT does a shift>>7 before indexing.
	angle0.raw += 1 << 6;
	angle1.raw += 1 << 6;

	// Reconstruct clipped vertices
	auto sin0 = intp12::castFromShiftedInteger<12>(lu_sin(angle0.raw));
	auto sin1 = intp12::castFromShiftedInteger<12>(lu_sin(angle1.raw));
	auto cos0 = intp12::castFromShiftedInteger<12>(lu_cos(angle0.raw));
	auto cos1 = intp12::castFromShiftedInteger<12>(lu_cos(angle1.raw));
#if FOV == 66 // Multiply by two to compensate for the 0.5 tan
	ndcA.x() = 2*cos0/sin0;
	ndcB.x() = 2*cos1/sin1;
#else
	dbgAssert(false); // Unimplemented FOV. Need to divide by tan(fov/2)
#endif

	// Depth calculation
	// We could safely cast down to .8 without loss because maps are grid aligned to .5 anyway.
	intp16 dx = (v1.x() - v0.x()).cast<16>();
	intp16 dy = (v1.y() - v0.y()).cast<16>();
	// Angle normal to the plane, in world space
	constexpr auto ang90deg = 0.25_p16;
	intp16 wsNormalAngle = fastAtan2(dx, dy) + ang90deg;
	// Angle from the shortest distance (aligned with the normal) to the reference angle
	intp16 offsetAngle = abs(rw_angle - wsNormalAngle);
	// Distance to the reference angle
	intp16 hyp = PointToDist(v0);
	// Project distance towards the normal
	intp16 proj = intp16::castFromShiftedInteger<12>(lu_cos(offsetAngle.raw));
	proj = max(proj, 0_p16);
	intp16 distanceToPlane = hyp * proj;

	// Move clipped angle to world space and substract 
	angle0 += camAngle;
	angle1 += camAngle;
	intp16 offset0 = angle0 - wsNormalAngle;
	intp16 offset1 = angle1 - wsNormalAngle;

	intp16 invD0 = intp16::castFromShiftedInteger<12>(max(0,lu_cos(offset0.raw))) / (distanceToPlane * sin0.cast<16>());
	intp16 invD1 = intp16::castFromShiftedInteger<12>(max(0,lu_cos(offset1.raw))) / (distanceToPlane * sin1.cast<16>());
	ndcA.y() = invD0.cast<12>();
	ndcB.y() = invD1.cast<12>();

	dbgAssert(ndcA.y() >= 0_p12);
	dbgAssert(ndcB.y() >= 0_p12);
	return true;
}

void clear(uint16_t* buffer, uint16_t topClr, uint16_t bottomClr, int area)
{
	DMA::Channel0().Fill(&buffer[0 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[1 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[2 * area / 4], bottomClr, area / 4);
	DMA::Channel0().Fill(&buffer[3 * area / 4], bottomClr, area / 4);
}

// Clip a segment against the screen.
// Returns whether the segment is potentially visible, and if so, fills in the clipped vertices into ndcA and ndcB.
// The clipped vertices have the following components:
// x: screen space x, in the range [-1,1]
// y: inverse distance to the camera plane.
bool clipSegment(const Pose& view, const WAD::Vertex* vertices, const WAD::Seg& segment, Vec2p12& ndcA, Vec2p12& ndcB)
{
	// Reconstruct segment vertices
	auto& v0 = vertices[segment.startVertex];
	auto& v1 = vertices[segment.endVertex];

	auto pos16 = Vec2p16(view.pos.m_y.cast<16>(), view.pos.m_y.cast<16>());
	// Project to view space
	auto vsA = v0 - pos16;
	auto vsB = v1 - pos16;

	// Clip
	return clipWall(vsA, vsB, view.phi, ndcA, ndcB);
}

void SectorRasterizer::RenderSubsector(const WAD::LevelData& level, uint16_t ssIndex, const Pose& view, DepthBuffer& depthBuffer)
{
	constexpr uint16_t FlagTwoSided = 0x04;
	const WAD::SubSector& subSector = level.subSectors[ssIndex];
	for (int i = subSector.firstSegment; i < subSector.firstSegment + subSector.segmentCount; ++i)
	{
		auto& segment = level.segments[i];

		Vec2p12 vA, vB;
		if (!clipSegment(view, level.vertices, segment, vA, vB))
		{
			continue; // Ignore non-visible segments
		}

		// Locate drawing info
		auto clrNdx = segment.linedefNum % 8;
		auto& lineDef = level.lineDefs[segment.linedefNum];
		auto& frontSide = level.sideDefs[lineDef.SideNum[0]];
		auto& frontSector = level.sectors[frontSide.sector];

		intp12 floorH = (intp16::castFromShiftedInteger<8>(frontSector.floorhHeight.raw) - view.pos.m_z).cast<12>();
		intp12 ceilingH = (intp16::castFromShiftedInteger<8>(frontSector.ceilingHeight.raw) - view.pos.m_z).cast<12>();

		if (lineDef.SideNum[1] == uint16_t(-1) // No back sector, must be an opaque wall
			|| !(lineDef.flags & FlagTwoSided)) // Explicitly opaque
		{
			RenderWall(vA, vB, floorH, ceilingH, edgeClr[clrNdx], depthBuffer);
			continue;
		}

		auto& backSide = level.sideDefs[lineDef.SideNum[1]];
		auto& backSector = level.sectors[backSide.sector];

		// TODO: Closed door optimization
		//if (backSector.ceilingHeight <= frontSector.floorhHeight
		//	|| backSector.floorhHeight >= frontSector.ceilingHeight)
		//{
		//	RenderWall(cam, vA, vB, floorH, ceilingH, edgeClr[clrNdx], depthBuffer);
		//	continue;
		//}

		// Invisible portal
		if (backSector.floorhHeight == frontSector.floorhHeight
			&& backSector.ceilingHeight == frontSector.ceilingHeight)
		{
			continue;
		}

		// Regular portal
		auto renderClr = segment.direction ? BasicColor::DarkGreen : edgeClr[clrNdx];
		RenderPortal(view, vA, vB, floorH, ceilingH, backSector, renderClr, depthBuffer);
	}
}

int32_t side(const WAD::Plane& plane, const intp16& x, const intp16& y)
{
	intp12 relX = (x - plane.origin.m_x).cast<12>();
	intp12 relY = (y - plane.origin.m_y).cast<12>();

	// It is safe to cast the plane component down to .8 without loss because we know they've been shifted on decompression
	auto cross = relX * plane.dir.m_y.cast<8>() - relY * plane.dir.m_x.cast<8>();
	// We just care about the sign, so ignore the shift
	return cross.raw > 0 ? 0 : 1;
}

bool insideAABB(const WAD::AABB& aabb, const Vec3p8& pos)
{
	return (pos.m_x.raw >= aabb.left.raw)
		&& (pos.m_x.raw <= aabb.right.raw)
		&& (pos.m_y.raw <= aabb.top.raw)
		&& (pos.m_y.raw >= aabb.bottom.raw);
}

void SectorRasterizer::RenderBSPNode(const WAD::LevelData& level, uint16_t nodeIndex, const Pose& view, DepthBuffer& depthBuffer)
{
	constexpr uint16_t NodeMask = (1 << 15);
	auto& node = level.nodes[nodeIndex];

	if (nodeIndex & NodeMask) // Leaf
	{
		// Render
		RenderSubsector(level, nodeIndex & ~NodeMask, view, depthBuffer);
		return;
	}
	else // Branch
	{
		// Traverse front to back
		int frontChild = side(node.plane, view.pos.m_x, view.pos.m_y);

		// Render the node I'm in first
		RenderBSPNode(level, node.child[frontChild], view, depthBuffer);

		// Then the node I'm not in
		// TODO: Check bounding box here
		RenderBSPNode(level, node.child[frontChild ^ 1], view, depthBuffer);
	}
}

void SectorRasterizer::RenderWorld(WAD::LevelData& level, const Camera& cam)
{
	// Since we always render front to back, we just need to keep track of whether a column has already been drawn or not.
	DepthBuffer depthBuffer;
	depthBuffer.Clear();

	// Traverse the BSP (in a random order for now)
	// Always start at the last node
	uint16_t rootNode = uint16_t(level.numNodes) - uint16_t(1);
	RenderBSPNode(level, rootNode, cam.m_pose, depthBuffer);
}

void SectorRasterizer::RenderWall(
	const Vec2p12& ndcA, const Vec2p12& ndcB,
	math::intp12 floorH, math::intp12 ceilingH,
	Color wallClr, DepthBuffer& depthBuffer)
{
	intp12 ssA = (ndcA.x() * int(DisplayMode::Width/2)) + int(DisplayMode::Width/2);
	intp12 ssB = (ndcB.x() * int(DisplayMode::Width/2)) + int(DisplayMode::Width/2);

	// No intersection with the view frustum
	int32_t x0 = ssA.floor();
	int32_t x1 = ssB.floor() + 1;
	if((x0 >= int32_t(DisplayMode::Width)) || (x1 < 0) || x0 >= x1)
	{
		return;
	}

	intp8 hFloorA = (floorH * ndcA.y()).cast<8>() * int(DisplayMode::Width/2);
	intp8 hFloorB = (floorH * ndcB.y()).cast<8>() * int(DisplayMode::Width/2);
	intp8 hCeilingA = (ceilingH * ndcA.y()).cast<8>() * int(DisplayMode::Width/2);
	intp8 hCeilingB = (ceilingH * ndcB.y()).cast<8>() * int(DisplayMode::Width/2);
	intp8 mFloor = (hFloorB - hFloorA) / (x1 - x0);
	intp8 mCeil = (hCeilingB - hCeilingA) / (x1 - x0);
	int y0A = (DisplayMode::Height / 2 - hCeilingA).floor();
	int y1A = (DisplayMode::Height / 2 - hFloorA).floor();
	
	x1 = std::min<int32_t>(x1, DisplayMode::Width-1);

	uint16_t* backbuffer = (uint16_t*)DisplayMode::backBuffer();
	for(int x = std::max<int32_t>(0,x0); x < x1; ++x)
	{		
		int floorDY = (mFloor * (x - x0)).floor();
		int ceilDY = (mCeil * (x - x0)).floor();

		int floorClip = depthBuffer.floorClip[x];
		int ceilingClip = depthBuffer.ceilingClip[x];
		if (ceilingClip >= floorClip)
		{
			continue;
		}

		int y0 = std::max<int32_t>(0, y0A - ceilDY);
		int y1 = std::min<int32_t>(DisplayMode::Height, y1A - floorDY);

		// Ceiling
		for(int y = ceilingClip; y < min(y0,floorClip); ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = skyClr;
		}

		// Wall		
		for(int y = max(y0, ceilingClip); y < min(y1, floorClip); ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = wallClr.raw;
		}

		// Ground
		for(int y = max(y1, ceilingClip); y < floorClip; ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = groundClr;
		}

		depthBuffer.ceilingClip[x] = floorClip;
	}
}

void SectorRasterizer::RenderPortal(const Pose& view,
	const Vec2p12& ndcA, const Vec2p12& ndcB,
	intp12 floorH, intp12 ceilingH,
	const WAD::Sector& backSector,
	Color wallClr, DepthBuffer& depthBuffer)
{
	// TODO: Use .12 precision here and move this into the clipping method instead?
	intp8 ssA = (ndcA.x() * int(DisplayMode::Width / 2)).cast<8>() + int(DisplayMode::Width / 2);
	intp8 ssB = (ndcB.x() * int(DisplayMode::Width / 2)).cast<8>() + int(DisplayMode::Width / 2);

	// No intersection with the view frustum
	int32_t x0 = ssA.floor();
	int32_t x1 = ssB.floor() + 1;
	if ((x0 >= int32_t(DisplayMode::Width)) || (x1 < 0) || x0 >= x1)
	{
		return;
	}

	// back sector heights
	intp12 backCeiling = backSector.ceilingHeight.cast<12>() - view.pos.m_z.cast<12>();
	intp12 backFloor = backSector.floorhHeight.cast<12>() - view.pos.m_z.cast<12>();

	intp8 hBakcFloorA = (backFloor * ndcA.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 hBakcFloorB = (backFloor * ndcB.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 hBakcCeilingA = (backCeiling * ndcA.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 hBakcCeilingB = (backCeiling * ndcB.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 mBackFloor = (hBakcFloorB - hBakcFloorA) / (x1 - x0);
	intp8 mBackCeil = (hBakcCeilingB - hBakcCeilingA) / (x1 - x0);

	// Front sector lines
	intp8 hFloorA = (floorH * ndcA.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 hFloorB = (floorH * ndcB.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 hCeilingA = (ceilingH * ndcA.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 hCeilingB = (ceilingH * ndcB.y()).cast<8>() * int(DisplayMode::Width / 2);
	intp8 mFloor = (hFloorB - hFloorA) / (x1 - x0);
	intp8 mCeil = (hCeilingB - hCeilingA) / (x1 - x0);

	// Draw limits on the first vertex
	int y0A = (DisplayMode::Height / 2 - hCeilingA).floor();
	int y1A = (DisplayMode::Height / 2 - hBakcCeilingA).floor(); // End of top
	int y2A = (DisplayMode::Height / 2 - hBakcFloorA).floor(); // Start of bottom
	int y3A = (DisplayMode::Height / 2 - hFloorA).floor();

	x1 = std::min<int32_t>(x1, DisplayMode::Width - 1);

	uint16_t* backbuffer = (uint16_t*)DisplayMode::backBuffer();
	for (int x = std::max<int32_t>(0, x0); x < x1; ++x)
	{
		int floorClip = depthBuffer.floorClip[x];
		int ceilingClip = depthBuffer.ceilingClip[x];
		// Skip fully occluded columns
		if (ceilingClip >= floorClip)
		{
			continue;
		}

		// Compute deltas for all 4 lines
		int floorDY = (mFloor * (x - x0)).floor();
		int ceilDY = (mCeil * (x - x0)).floor();
		int backFloorDY = (mBackFloor * (x - x0)).floor();
		int backCeilDY = (mBackCeil * (x - x0)).floor();

		// Draw the ceiling in front;

		int32_t y0 = y0A - ceilDY;
		for (int y = ceilingClip; y < min(y0, floorClip); ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = skyClr;
		}

		// Draw the top section
		int y1 = y1A - backCeilDY;
		for (int y = max(y0, ceilingClip); y < min(y1, floorClip); ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = wallClr.raw;
		}
		depthBuffer.ceilingClip[x] = max(ceilingClip, y1);

		// Bottom wall
		int y2 = y2A - backFloorDY;
		int y3 = y3A - floorDY;
		for (int y = max(y2, ceilingClip); y < min(y3, floorClip); ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = wallClr.raw;
		}
		depthBuffer.floorClip[x] = max(0, min(floorClip, y3));

		// Ground
		for (int y = max(y3, ceilingClip); y < floorClip; ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = groundClr;
		}
	}
}

void SectorRasterizer::DepthBuffer::Clear()
{
	for(int i = 0; i < DisplayMode::Width; ++i)
	{
		ceilingClip[i] = 0;
		floorClip[i] = DisplayMode::Height;
	}
}