#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t e1m1_WADVerticesSize = 1884;
extern const uint32_t e1m1_WADVertices[];

constexpr uint32_t e1m1_WADLineDefsSize = 3616;
extern const uint32_t e1m1_WADLineDefs[];

constexpr uint32_t e1m1_WADSideDefsSize = 9923;
extern const uint32_t e1m1_WADSideDefs[];

constexpr uint32_t e1m1_WADSegmentsSize = 4389;
extern const uint32_t e1m1_WADSegments[];

constexpr uint32_t e1m1_WADSubSectorsSize = 448;
extern const uint32_t e1m1_WADSubSectors[];

constexpr uint32_t e1m1_WADSectorsSize = 1300;
extern const uint32_t e1m1_WADSectors[];

constexpr uint32_t e1m1_WADNodesSize = 3129;
extern const uint32_t e1m1_WADNodes[];

void loadMap_e1m1_WAD(WAD::LevelData& dstLevel);

