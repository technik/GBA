#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t mercury_WADVerticesSize = 1040;
extern const uint32_t mercury_WADVertices[];

constexpr uint32_t mercury_WADLineDefsSize = 1782;
extern const uint32_t mercury_WADLineDefs[];

constexpr uint32_t mercury_WADSideDefsSize = 6308;
extern const uint32_t mercury_WADSideDefs[];

constexpr uint32_t mercury_WADSegmentsSize = 2772;
extern const uint32_t mercury_WADSegments[];

constexpr uint32_t mercury_WADSubSectorsSize = 287;
extern const uint32_t mercury_WADSubSectors[];

constexpr uint32_t mercury_WADSectorsSize = 663;
extern const uint32_t mercury_WADSectors[];

constexpr uint32_t mercury_WADNodesSize = 2574;
extern const uint32_t mercury_WADNodes[];

void loadMap_mercury_WAD(WAD::LevelData& dstLevel);

