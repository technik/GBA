//
// Performance critical code that need to be in IWRAM to keep 
// Sector rasterizer performance
//

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>
#include <SectorRasterizer.h>

#include <gfx/palette.h>

using namespace math;
using namespace gfx;

//Vec2p8 vertices[] = {
//	{ 1_p8, 6_p8 },
//	{ 4_p8, 6_p8 },
//	{ 5_p8, 0_p8 },
//	{ 3_p8, 0_p8 },
//	{ 1_p8, 6_p8 },
//};

Vec2p8 vertices[] = {
	{ -2_p8, 2_p8 },
	{  2_p8, 2_p8 },
	{  2_p8,-2_p8 },
	{ -2_p8,-2_p8 }
};

// TODO: Optimize this with a LUT to avoid the BIOS call
intp16 fastAtan2(intp8 x, intp8 y)
{
	// Map angle to the first quadrant
	intp8 x1 = abs(x);
	intp8 y1 = abs(y);

	// Note: ArcTan takes a .14 fixed argument. To get the best precision, we perform the division shifts manually
	intp16 atan16;
	if (x1 >= abs(y1)) // Lower half of Q1, upper half of Q4
	{
		// .14f = ( .8f << 14 ) / .8f
		int ratio = ((y.raw << 14) / x1.raw); // Note we used y with sign to get both the tangent of the first and second quadrants correctly
		atan16 = intp16::castFromShiftedInteger<16>(ArcTan(ratio));
	}
	else // Upper half of Q1, lower half of Q4
	{
		// .14f = ( .8f << 14 ) / .8f
		int ratio = ((x1.raw << 14) / y1.raw);
		atan16 = 0.25_p16 - intp16::castFromShiftedInteger<16>(ArcTan(ratio));
		atan16 = y > 0 ? atan16 : -atan16;
	}
	return (x.raw >= 0) ? atan16 : 0.5_p16 - atan16;
}

Vec2p8 worldToViewSpace(const Pose& camPose, const Vec2p8& worldPos)
{
	Vec2p8 relPos;
	relPos.x() = worldPos.x() - camPose.pos.x();
	relPos.y() = worldPos.y() - camPose.pos.y();

	Vec2p8 viewSpace;
	viewSpace.x() = (relPos.x() * camPose.cosf + relPos.y() * camPose.sinf).cast<8>();
	viewSpace.y() = (relPos.y() * camPose.cosf - relPos.x() * camPose.sinf).cast<8>();

	return viewSpace;
}

// Returns x in screen space, invDepth as y
Vec2p12 viewToClipSpace(const Vec2p8& viewSpace, int halfScreenWidth)
{
	// Project x onto the screen
	// Assuming tg(fov_x/2) = 0.5
	intp12 invDepth = 2_p12 / max(intp12(1.f/64), viewSpace.y().cast<12>());
	Vec2p12 result;
	result.x() = (viewSpace.x() * invDepth.cast<8>()).cast<12>();
	result.y() = invDepth;
	return result;
}

// Clips a wall that's already in view space.
// Returns whether the wall is visible.
bool clipWall(Vec2p8& v0, Vec2p8& v1)
{
	constexpr intp8 nearClip = intp8(1/64.f);
	// Clip behind the view.
	if (v0.y() <= nearClip && v1.y() <= nearClip)
		return false;

	// Clip against the y=0 line
	// Skip walls fully positive or parallel to y=0
	if((v0.y() < nearClip ||  v1.y() < nearClip) && !(v0.y() == v1.y()))
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
	if(v0.x() * v1.y() >= v1.x() * v0.y())
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

void SectorRasterizer::RenderWorld(const Camera& cam)
{
	uint16_t* backbuffer = (uint16_t*)DisplayMode::backBuffer();

	// Clean the background
	DMA::Channel0().Fill(&backbuffer[0*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[1*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[2*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[3*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);

	RenderWall(cam, vertices[0], vertices[1], BasicColor::Green);
	RenderWall(cam, vertices[1], vertices[2], BasicColor::DarkGreen);
	RenderWall(cam, vertices[2], vertices[3], BasicColor::LightGrey);
	RenderWall(cam, vertices[3], vertices[0], BasicColor::Yellow);
}

void SectorRasterizer::RenderWall(const Camera& cam, const Vec2p8& A, const Vec2p8& B, Color wallClr)
{
	uint16_t* backbuffer = (uint16_t*)DisplayMode::backBuffer();

	auto vsA = worldToViewSpace(cam.m_pose, A);
	auto vsB = worldToViewSpace(cam.m_pose, B);

	// Clip
	if(!clipWall(vsA, vsB))
		return; // Early out on clipped walls

	Vec2p12 csA = viewToClipSpace(vsA, DisplayMode::Width/2);
	Vec2p12 csB = viewToClipSpace(vsB, DisplayMode::Width/2);

	// TODO: Clip wall to the view frustum and recompute invDepth from there
	intp8 ssA = (csA.x() * int(DisplayMode::Width/2)).cast<8>() + int(DisplayMode::Width/2);
	intp8 ssB = (csB.x() * int(DisplayMode::Width/2)).cast<8>() + int(DisplayMode::Width/2);

	// No intersection with the view frustum
	int32_t x0 = ssA.floor();
	int32_t x1 = ssB.floor() + 1;
	if((x0 >= int32_t(DisplayMode::Width)) || (x1 < 0) || x0 >= x1)
	{
		return;
	}

	intp8 h0 = (int32_t(DisplayMode::Width/2) * csA.y()).cast<8>();
	intp8 h1 = (int32_t(DisplayMode::Width/2) * csB.y()).cast<8>();
	intp8 m = (h0-h1) / (x1-x0);
	
	x1 = std::min<int32_t>(x1, DisplayMode::Width-1);

	for(int x = std::max<int32_t>(0,x0); x < x1; ++x)
	{		
		int mx = (m*(x-x0)).floor();
		int y0 = std::max<int32_t>(0, (DisplayMode::Height/2 - h0).floor() + mx);
		int y1 = std::min<int32_t>(DisplayMode::Height, (DisplayMode::Height/2 + h0).floor() - mx);
				
		for(int y = y0; y < y1; ++y)
		{
			auto pixel = DisplayMode::pixel(x, y);
			backbuffer[pixel] = wallClr.raw;
		}
	}
}