//
// Performance critical code that need to be in IWRAM to keep 
// Sector rasterizer performance
//
extern "C" {
	#include <tonc.h>
}

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>
#include <SectorRasterizer.h>

#include <gfx/palette.h>

using namespace math;
using namespace gfx;

Vec2p8 vertices[] = {
	{ 1_p8, 6_p8 },
	{ 4_p8, 6_p8 },
	{ 5_p8, 9_p8 },
	{ 3_p8, 7_p8 }
};

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

// Clips a wall that's already in view space.
// Returns whether the wall is visible.
bool clipWall(Vec2p8& v0, Vec2p8& v1)
{
	// Clip backfaces
	if(v0.x() > v1.x())
		return false;
	
	// Clip behind the view. TODO: Support a non-zero near clip
	if(v0.y() <= 0 && v1.y() <= 0)
		return false;

	// Early out for no clipping
	if(v0.y() > 0 && v1.y() > 0)
		return true;

	if(v0.y() == v1.y()) // Nothing to clip here, the plane is parallel to the y=0 plane, so it can't intersect it.
		return true;

	Vec2p8 dWall = v1 - v0;
	intp8 y0 = max(0_p8, v0.y());
	intp8 y1 = max(0_p8, v1.y());

	intp12 denom = (v1.y() - v0.y()).cast<12>();

	intp12 num0 = (dWall.x() * (y0 - v0.y())).cast<12>();
	intp12 num1 = (dWall.x() * (y1 - v1.y())).cast<12>();

	v0.x() = v0.x() + (num0 / denom).cast<8>();
	v1.x() = v1.x() + (num1 / denom).cast<8>();
	v0.y() = y0;
	v1.y() = y1;

	return true;
}

void SectorRasterizer::RenderWorld(const Camera& cam)
{
	uint16_t* backbuffer = DisplayControl::Get().backBuffer();

	// Clean the background
	DMA::Channel0().Fill(&backbuffer[0*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[1*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[2*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[3*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);

	RenderWall(cam, vertices[0], vertices[1], BasicColor::Green);
	RenderWall(cam, vertices[1], vertices[2], BasicColor::DarkGreen);
	RenderWall(cam, vertices[2], vertices[3], BasicColor::LightGrey);
	RenderWall(cam, vertices[3], vertices[4], BasicColor::Yellow);
}

void SectorRasterizer::RenderWall(const Camera& cam, const Vec2p8& A, const Vec2p8& B, Color wallClr)
{
	uint16_t* backbuffer = DisplayControl::Get().backBuffer();

	auto vsA = worldToViewSpace(cam.m_pose, A);
	auto vsB = worldToViewSpace(cam.m_pose, B);

	if(!clipWall(vsA, vsB))
		return; // Early out on clipped walls

	// Draw a wall
	Vec3p8 ssA = cam.projectWorldPos2D(A);
	Vec3p8 ssB = cam.projectWorldPos2D(B);

	// No intersection with the view frustum
	int32_t x0 = ssA.x().floor();
	int32_t x1 = ssB.x().floor();
	if((x0 >= int32_t(DisplayMode::Width)) || (x1 < 0) || x0 >= x1)
	{
		return;
	}

	intp8 h0 = int32_t(DisplayMode::Height/2) * ssA.z();
	intp8 h1 = int32_t(DisplayMode::Height/2) * ssB.z();
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