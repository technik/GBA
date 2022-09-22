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

using namespace math;
using namespace gfx;

// No need to place this method in fast memory
void SectorRasterizer::Init()
{
	DisplayMode displayMode;
	displayMode.Init();
	Display().enableSprites();
}

bool loadWAD(WAD::LevelData& dstLevel)
{
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

    return true;
}