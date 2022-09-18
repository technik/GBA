// Implement a ray tracer that can be used both on PC or in the GBA
// Uses fixed point math, dithering and color palettes for compatibility
// with the GBA, but can easily be debugged on PC.

#include <linearMath.h>
#include <Camera.h>
#include <Display.h>
#include <cassert>
#include <imageUtils.h>

#include <../../raycaster/include/SectorRasterizer.h>

// Embedded assets
#include <mercury.wad.h>
#include <test.wad.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace math;

static constexpr size_t LevelSize = sizeof(LevelData);

bool loadWAD(LevelData& dstLevel, const uint32_t* wadData)
{
    const uint8_t* byteData = reinterpret_cast<const uint8_t*>(wadData);
    // Verify WAD format
    const char* defString = reinterpret_cast<const char*>(wadData);
    assert(!strncmp(defString, "PWAD", 4));
    if (strncmp(defString, "PWAD", 4) != 0)
        return false;

    auto wadHeader = reinterpret_cast<const WAD::Header*>(wadData);

    auto directory = reinterpret_cast<const WAD::Lump*>(&byteData[wadHeader->dirOffset]);

    // Lump directories
    auto& mapName = directory[0];
    auto& things = directory[1];
    auto& lineDefsLump = directory[2];
    auto& sideDefsLump = directory[3];
    auto& verticesLump = directory[4];
    auto& segLumps = directory[5];
    auto& ssectorLumps = directory[6];
    auto& nodeLumps = directory[7];
    auto& sectorLumps = directory[8];
    auto& reject = directory[9];
    auto& blockMap = directory[10];

    // Load vertex data
    if (verticesLump.dataSize > LevelData::MAX_VERTICES * sizeof(WAD::Vertex))
    {
        return false; // Too many vertices in the map
    }
    
    int numVertices = verticesLump.dataSize / sizeof(WAD::Vertex);
    memcpy(dstLevel.vertices, &byteData[verticesLump.dataOffset], verticesLump.dataSize);

    // TODO: Scale down vertices to a more reasonable size than the default one in the editor

    // Load line defs
    if (lineDefsLump.dataSize > LevelData::MAX_LINEDEFS * sizeof(WAD::LineDef))
    {
        return false; // Too many linedefs in the map
    }

    int numLineDefs = lineDefsLump.dataSize / sizeof(WAD::LineDef);
    const auto* srcLineDefs = (WAD::LineDef*)&byteData[lineDefsLump.dataOffset];
    memcpy(dstLevel.linedefs, srcLineDefs, lineDefsLump.dataSize);

    // Load nodes
    if (nodeLumps.dataSize > LevelData::MAX_NODES * sizeof(WAD::Node))
    {
        return false; // Too many nodes in the map
    }

    dstLevel.numNodes = nodeLumps.dataSize / sizeof(WAD::Node);
    const auto* srcNodes = (WAD::Node*)&byteData[nodeLumps.dataOffset];
    memcpy(dstLevel.nodes, srcNodes, nodeLumps.dataSize);

    // Load subsectors
    if (ssectorLumps.dataSize > LevelData::MAX_SUBSECTORS * sizeof(WAD::SubSector))
    {
        return false; // Too many subsectors in the map
    }

    const auto* srcSubSectors = (WAD::SubSector*)&byteData[ssectorLumps.dataOffset];
    memcpy(dstLevel.subsectors, srcSubSectors, ssectorLumps.dataSize);

    // Load segments
    if (segLumps.dataSize > LevelData::MAX_SEGMENTS * sizeof(WAD::Seg))
    {
        return false; // Too many segments in the map
    }

    const auto* srcSegs = (WAD::Seg*)&byteData[segLumps.dataOffset];
    memcpy(dstLevel.segments, srcSegs, segLumps.dataSize);

    // Load sectors
    if (sectorLumps.dataSize > LevelData::MAX_SECTORS * sizeof(WAD::Sector))
    {
        return false; // Too many sectors in the map
    }

    const auto* srcSectors = (WAD::Sector*)&byteData[sectorLumps.dataOffset];
    memcpy(dstLevel.sectors, srcSectors, sectorLumps.dataSize);

    return true;
}

int main()
{
    Mode5Display displayMode; 
    displayMode.Init();

    // Load a WAD map
    LevelData level;
    loadWAD(level, test_WAD);
    //loadWAD(mercury_WAD);

    // Create a camera
    Camera cam(Mode5Display::Width, Mode5Display::Height, Vec3p8(0_p8, 0_p8, 0_p8));

    // Debug test pose
    cam.m_pose.phi.raw = 0;// 39300;
    cam.m_pose.pos.m_x.raw = -48;
    cam.m_pose.pos.m_y.raw = -96;
    cam.m_pose.update();

    // Render
    SectorRasterizer::Init();

    SectorRasterizer::RenderWorld(level, cam);
    
    // Save the image
    Mode5Display::s_backBuffer.save("render.png");
    return 0;
}