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



void SectorRasterizer::RenderWorld(const Camera& cam)
{
	uint16_t* backbuffer = DisplayControl::Get().backBuffer();

	DMA::Channel0().Fill(backbuffer, &fillClr, 3*DisplayMode::Area/4);
}