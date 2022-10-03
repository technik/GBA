#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t mercury_WADVerticesSize = 694;
extern const uint32_t mercury_WADVertices[];

constexpr uint32_t mercury_WADLineDefsSize = 1176;
extern const uint32_t mercury_WADLineDefs[];

constexpr uint32_t mercury_WADSideDefsSize = 3825;
extern const uint32_t mercury_WADSideDefs[];

constexpr uint32_t mercury_WADSegmentsSize = 1698;
extern const uint32_t mercury_WADSegments[];

constexpr uint32_t mercury_WADSubSectorsSize = 176;
extern const uint32_t mercury_WADSubSectors[];

constexpr uint32_t mercury_WADSectorsSize = 397;
extern const uint32_t mercury_WADSectors[];

constexpr uint32_t mercury_WADNodesSize = 1575;
extern const uint32_t mercury_WADNodes[];

void loadMap_mercury_WAD(WAD::LevelData& dstLevel);

