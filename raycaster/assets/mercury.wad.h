#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t mercury_WADVerticesSize = 162;
extern const uint32_t mercury_WADVertices[];

constexpr uint32_t mercury_WADLineDefsSize = 648;
extern const uint32_t mercury_WADLineDefs[];

constexpr uint32_t mercury_WADSideDefsSize = 2625;
extern const uint32_t mercury_WADSideDefs[];

constexpr uint32_t mercury_WADSegmentsSize = 1113;
extern const uint32_t mercury_WADSegments[];

constexpr uint32_t mercury_WADSubSectorsSize = 108;
extern const uint32_t mercury_WADSubSectors[];

constexpr uint32_t mercury_WADSectorsSize = 286;
extern const uint32_t mercury_WADSectors[];

constexpr uint32_t mercury_WADNodesSize = 749;
extern const uint32_t mercury_WADNodes[];

void loadMap_mercury_WAD(WAD::LevelData& dstLevel);

