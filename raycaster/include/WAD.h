#pragma once

#include <linearMath.h>
#include <vector.h>

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

    // Parsed WAD
    struct LevelData
    {
        uint32_t numNodes = 0;

        const WAD::Vertex* vertices;
        const WAD::LineDef* linedefs;
        const WAD::Node* nodes;
        const WAD::Seg* segments;
        const WAD::Sector* sectors;
        const WAD::SubSector* subsectors;
    };
}
