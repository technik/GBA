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

#define LEVEL 0

bool loadWAD(WAD::LevelData& dstLevel)
{
#if LEVEL == 0
    // Load vertex data
    dstLevel.vertices = (const WAD::Vertex*)test_WADVertices;

    // Load line defs
    dstLevel.linedefs = (const WAD::LineDef*)test_WADLineDefs;

    // Load nodes
    dstLevel.numNodes = (test_WADNodesSize*4) / sizeof(WAD::Node);
    dstLevel.nodes = (const WAD::Node*)test_WADNodes;

    // Load subsectors
    dstLevel.subsectors = (const WAD::SubSector*)test_WADSubsectors;

    // Load segments
    dstLevel.segments = (const WAD::Seg*)test_WADSegments;

    // Load sectors
    dstLevel.sectors = (const WAD::Sector*)test_WADSectors;
#elif LEVEL == 1
    // Load vertex data
    dstLevel.vertices = (const WAD::Vertex*)portaltest_WADVertices;

    // Load line defs
    dstLevel.linedefs = (const WAD::LineDef*)portaltest_WADLineDefs;

    // Load nodes
    dstLevel.numNodes = (portaltest_WADNodesSize * 4) / sizeof(WAD::Node);
    dstLevel.nodes = (const WAD::Node*)portaltest_WADNodes;

    // Load subsectors
    dstLevel.subsectors = (const WAD::SubSector*)portaltest_WADSubsectors;

    // Load segments
    dstLevel.segments = (const WAD::Seg*)portaltest_WADSegments;

    // Load sectors
    dstLevel.sectors = (const WAD::Sector*)portaltest_WADSectors;
#elif LEVEL == 2
#endif

    return true;
}