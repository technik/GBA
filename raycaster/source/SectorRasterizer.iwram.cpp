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

Vec2p8 pointA = { 1_p8, 4_p8 };
Vec2p8 pointB = { 4_p8, 4_p8 };

void SectorRasterizer::RenderWorld(const Camera& cam)
{
	uint16_t* backbuffer = DisplayControl::Get().backBuffer();

	// Clean the background
	//for(int i = 0; i < DisplayMode::Height/2; ++i)
	//{
	//	DMA::Channel0().Fill(&backbuffer[2*i*DisplayMode::Width], &fillClr, DisplayMode::Width*2);
	//}

	// Draw a wall
	Vec3p8 ssA = cam.projectWorldPos(pointA);
	Vec3p8 ssB = cam.projectWorldPos(pointB);

	if(ssA.y() <= 0 && ssB.y() <= 0) // Fully clipped, behind the camera
	{
		return;
	}

	// No intersection with the view frustum
	int x0 = ssA.x().floor();
	int x1 = ssB.x().floor();
	if((x0 >= DisplayMode::Width) || (x1 < 0))
	{
		return;
	}

	int ssX = ssA.x().roundToInt();
	intp8 h0 = int(DisplayMode::Height/2) * ssA.z();
	intp8 h1 = int(DisplayMode::Height/2) * ssB.z();
	intp8 dy = (h1-h0) / x1-x0;

	for(int x = x0; x < x1; ++x)
	{
		int m = (dy*(x-x0)).floor();
		int y0 = (DisplayMode::Height/2 - h0).floor() + m;
		int y1 = (DisplayMode::Height/2 + h0).floor() - m;
		
		auto pixel = DisplayMode::pixel(x, y0);
		backbuffer[pixel] = BasicColor::LightGrey.raw;
		
		pixel = DisplayMode::pixel(x, y1);
		backbuffer[pixel] = BasicColor::LightGrey.raw;
		
		//for(int y = y0; y < y1; ++y)
		//{
		//	pixel = DisplayMode::pixel(x, y);
		//	backbuffer[pixel] = BasicColor::Green.raw;
		//}
	}

	if(ssX >= 0 && ssX < DisplayMode::Width)
	{
		auto halfH = (DisplayMode::Height/4 * ssA.z()).roundToInt();
		auto y0 = max(0, DisplayMode::Height/2 - halfH);
		auto y1 = min(DisplayMode::Height-1, DisplayMode::Height/2 + halfH);
		for(int i = y0; i < y1; ++i)
		{
			auto pixel = DisplayMode::pixel(ssX, i);
			backbuffer[pixel] = BasicColor::Blue.raw;
		}
	}

	ssX = ssB.x().roundToInt();
	if(ssX >= 0 && ssX < DisplayMode::Width )
	{
		auto halfH = (DisplayMode::Height/4 * ssB.z()).roundToInt();
		auto y0 = max(0, DisplayMode::Height/2 - halfH);
		auto y1 = min(DisplayMode::Height, DisplayMode::Height/2 + halfH);
		for(int i = y0; i < y1; ++i)
		{
			auto pixel = DisplayMode::pixel(ssX, i);
			backbuffer[pixel] = BasicColor::Red.raw;
		}
	}

	backbuffer[DisplayMode::Width/2] = BasicColor::Red.raw;
}