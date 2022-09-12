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

Vec2p8 pointA = { 1_p8, 6_p8 };
Vec2p8 pointB = { 4_p8, 6_p8 };

void SectorRasterizer::RenderWorld(const Camera& cam)
{
	uint16_t* backbuffer = DisplayControl::Get().backBuffer();

	// Clean the background
	DMA::Channel0().Fill(&backbuffer[0*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[1*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[2*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);
	DMA::Channel0().Fill(&backbuffer[3*DisplayMode::Area/4], fillClr, DisplayMode::Area/4);

	// Draw a wall
	Vec3p8 ssA = cam.projectWorldPos2D(pointA);
	Vec3p8 ssB = cam.projectWorldPos2D(pointB);

	if(ssA.y() <= 0 && ssB.y() <= 0) // Fully clipped, behind the camera
	{
		return;
	}

	// No intersection with the view frustum
	int32_t x0 = ssA.x().floor();
	int32_t x1 = ssB.x().floor();
	if((x0 >= int32_t(DisplayMode::Width)) || (x1 < 0) || x0 >= x1)
	{
		return;
	}

	int32_t ssX = ssA.x().roundToInt();
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
			backbuffer[pixel] = BasicColor::Green.raw;
		}
	}
}