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

// Maps
#include <test.wad.h>
#include <mercury.wad.h>
#include <portaltest.wad.h>
#include <e1m1.wad.h>

using namespace math;
using namespace gfx;

// No need to place this method in fast memory
void SectorRasterizer::Init()
{
	displayMode.Init();
	Display().enableSprites();
}

bool SectorRasterizer::BeginFrame()
{
    return displayMode.BeginFrame();
}

void SectorRasterizer::EndFrame()
{
    displayMode.Flip();
}

#define LEVEL 3

bool loadWAD(WAD::LevelData& dstLevel)
{
#if LEVEL == 0
    loadMap_test_WAD(dstLevel);
#elif LEVEL == 1
    loadMap_mercury_WAD(dstLevel);
#elif LEVEL == 2
    loadMap_portaltest_WAD(dstLevel);
#elif LEVEL == 3
    loadMap_e1m1_WAD(dstLevel);
#endif

    return true;
}