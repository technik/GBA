#pragma once
//
// m7_isrs.c
// Separate file for HBL interrupts because apparently it screws up 
//   on hardware now.
#include <Display.h>
#include <Camera.h>
#include <vector.h>
#include <Color.h>

#ifdef GBA
extern "C" {
#include <tonc.h>
}
#endif // GBA

// Original DOOM WAD types
namespace WAD
{

    struct Header
    {
        int32_t Signature;
        int32_t numLumps;
        int32_t dirOffset;
    };

    struct Lump
    {
        int32_t dataOffset;
        int32_t dataSize;
        char lumpName[8];
    };

    struct Vertex
    {
        math::int8p8 x, y;
    };

    struct LineDef
    {
        uint16_t v0, v1;
        uint16_t flags;
        uint16_t SpecialType;
        uint16_t SectorTag;
        uint16_t RightSideDef;
        uint16_t LeftSideDef;
    };

    struct Seg
    {
        int16_t startVertex;
        int16_t endVertex;
        int16_t angle;
        int16_t linedefNum;
        int16_t direction; // Binary flag (0: [v0,v1], 1: [v1,v0])
        math::int8p8 offset; // Offset along the linedef to the start of this seg
    };

    struct SubSector
    {
        int16_t segmentCount;
        int16_t firstSegment;
    };

    struct AABB
    {
        math::int8p8 top, bottom, left, right;
    };

    struct Node
    {
        math::int8p8 x, y;
        math::int8p8 dx, dy;
        AABB aabb[2];
        uint16_t child[2];
    };

    struct Sector
    {
        math::int8p8 floorhHeight;
        math::int8p8 ceilingHeight;
        char floorTextureName[8];
        char ceilingTextureName[8];
        math::int8p8 lightLevel;
        int16_t type;
        int16_t tagNumber;
    };
}

struct LevelData
{
    static constexpr int32_t MAX_VERTICES = 64;
    static constexpr int32_t MAX_LINEDEFS = 64;
    static constexpr int32_t MAX_NODES = 32;
    static constexpr int32_t MAX_SECTORS = 32;
    static constexpr int32_t MAX_SUBSECTORS = 64;
    static constexpr int32_t MAX_SEGMENTS = 64;

    int32_t numNodes = 0;

    WAD::Vertex vertices[MAX_VERTICES];
    WAD::LineDef linedefs[MAX_LINEDEFS];
    WAD::Node nodes[MAX_NODES];
    WAD::Seg segments[MAX_SEGMENTS];
    WAD::Sector sectors[MAX_SECTORS];
    WAD::SubSector subsectors[MAX_SUBSECTORS];
};

// Util
static constexpr size_t LevelDataSize = sizeof(LevelData);

bool loadWAD(LevelData& dstLevel, const uint32_t* wadData);

class SectorRasterizer
{
public:
    using DisplayMode = Mode5Display;

    static void Init();
    static void RenderWorld(LevelData& level, const Camera& cam);

    static inline uint16_t fillClr = BasicColor::SkyBlue.raw;
    static inline uint16_t fillClr2 = BasicColor::Red.raw;

private:
    static void RenderSubsector(const LevelData& level, uint16_t ssIndex, const Camera& cam);
    static void RenderBSPNode(const LevelData& level, uint16_t nodeIndex, const Camera& cam);
    static void RenderWall(const Camera& cam, const math::Vec2p8& A, const math::Vec2p8& B, Color clr);
};