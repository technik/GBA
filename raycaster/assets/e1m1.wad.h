#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t e1m1_WADVerticesSize = 2350;
extern const uint32_t e1m1_WADVertices[];

constexpr uint32_t e1m1_WADLineDefsSize = 998;
extern const uint32_t e1m1_WADLineDefs[];

constexpr uint32_t e1m1_WADSideDefsSize = 1710;
extern const uint32_t e1m1_WADSideDefs[];

constexpr uint32_t e1m1_WADSegmentsSize = 3864;
extern const uint32_t e1m1_WADSegments[];

constexpr uint32_t e1m1_WADSubSectorsSize = 0;
extern const uint32_t e1m1_WADSubSectors[];

constexpr uint32_t e1m1_WADSectorsSize = 3614;
extern const uint32_t e1m1_WADSectors[];

constexpr uint32_t e1m1_WADNodesSize = 651;
extern const uint32_t e1m1_WADNodes[];

void loadMap_e1m1_WAD(WAD::LevelData& dstLevel);

