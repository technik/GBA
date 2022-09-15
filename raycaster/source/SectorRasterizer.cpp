//
// Sector rasterizer code that doesn't need to fit in IWRAM
//

#include <Camera.h>
#include <raycaster.h>

#include <Color.h>
#include <Device.h>
#include <linearMath.h>

#include <gfx/palette.h>
#include <SectorRasterizer.h>

using namespace math;
using namespace gfx;

// No need to place this method in fast memory
void SectorRasterizer::Init()
{
	DisplayMode displayMode;
	displayMode.Init();
	Display().enableSprites();
}