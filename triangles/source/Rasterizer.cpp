//
// Sector rasterizer code that doesn't need to fit in IWRAM
//

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>

#include <gfx/palette.h>
#include <Rasterizer.h>

// Maps
#include <test.wad.h>
#include <mercury.wad.h>
#include <portaltest.wad.h>
#include <e1m1.wad.h>

using namespace math;
using namespace gfx;

void clear(uint16_t* buffer, uint16_t topClr, uint16_t bottomClr, int area)
{
	DMA::Channel0().Fill(&buffer[0 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[1 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[2 * area / 4], bottomClr, area / 4);
	DMA::Channel0().Fill(&buffer[3 * area / 4], bottomClr, area / 4);
}

// No need to place this method in fast memory
void Rasterizer::Init()
{
	displayMode.Init();
	Display().enableSprites();
}

bool Rasterizer::BeginFrame()
{
    return displayMode.BeginFrame();
}

void Rasterizer::EndFrame()
{
    displayMode.Flip();
}

void Rasterizer::RenderWorld(const Camera& cam)
{
	clear(Display().backBuffer(), skyClr.raw, groundClr.raw, displayMode.Area);
}