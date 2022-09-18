//
// Sector rasterizer code that doesn't need to fit in IWRAM
//

#include <Camera.h>
#include <cstring>
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

bool loadWAD(LevelData& dstLevel, const uint32_t* wadData)
{
    const uint8_t* byteData = reinterpret_cast<const uint8_t*>(wadData);
    // Verify WAD format
    const char* defString = reinterpret_cast<const char*>(wadData);
    //assert(!strncmp(defString, "PWAD", 4));
    if (strncmp(defString, "PWAD", 4) != 0)
        return false;

    auto wadHeader = reinterpret_cast<const WAD::Header*>(wadData);

    auto directory = reinterpret_cast<const WAD::Lump*>(&byteData[wadHeader->dirOffset]);

    // Lump directories
    //auto& mapName = directory[0];
    //auto& things = directory[1];
    //auto& lineDefsLump = directory[2];
    //auto& sideDefsLump = directory[3];
    auto& verticesLump = directory[4];
    //auto& segLumps = directory[5];
    //auto& ssectorLumps = directory[6];
    //auto& nodeLumps = directory[7];
    //auto& sectorLumps = directory[8];
    //auto& reject = directory[9];
    //auto& blockMap = directory[10];

    // Load vertex data
    dstLevel.vertices = reinterpret_cast<const WAD::Vertex*>(&byteData[verticesLump.dataOffset]);

    // Load line defs
    /*dstLevel.linedefs = (const WAD::LineDef*)(&byteData[lineDefsLump.dataOffset]);

    // Load nodes
    dstLevel.numNodes = nodeLumps.dataSize / sizeof(WAD::Node);
    dstLevel.nodes = (const WAD::Node*)(&byteData[nodeLumps.dataOffset]);

    // Load subsectors
    dstLevel.subsectors = (const WAD::SubSector*)&byteData[ssectorLumps.dataOffset];

    // Load segments
    dstLevel.segments = (const WAD::Seg*)(&byteData[segLumps.dataOffset]);

    // Load sectors
    dstLevel.sectors = (const WAD::Sector*)&byteData[sectorLumps.dataOffset];
*/
    return true;
}