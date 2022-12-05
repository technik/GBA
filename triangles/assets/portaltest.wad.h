#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t portaltest_WADVerticesSize = 144;
extern const uint32_t portaltest_WADVertices[];

constexpr uint32_t portaltest_WADLineDefsSize = 242;
extern const uint32_t portaltest_WADLineDefs[];

constexpr uint32_t portaltest_WADSideDefsSize = 728;
extern const uint32_t portaltest_WADSideDefs[];

constexpr uint32_t portaltest_WADSegmentsSize = 318;
extern const uint32_t portaltest_WADSegments[];

constexpr uint32_t portaltest_WADSubSectorsSize = 29;
extern const uint32_t portaltest_WADSubSectors[];

constexpr uint32_t portaltest_WADSectorsSize = 52;
extern const uint32_t portaltest_WADSectors[];

constexpr uint32_t portaltest_WADNodesSize = 252;
extern const uint32_t portaltest_WADNodes[];

void loadMap_portaltest_WAD(WAD::LevelData& dstLevel);

