#pragma once
#include <cstdint>
#include <base.h>
#include <WAD.h>

constexpr uint32_t test_WADVerticesSize = 24;
extern const uint32_t test_WADVertices[];

constexpr uint32_t test_WADLineDefsSize = 81;
extern const uint32_t test_WADLineDefs[];

constexpr uint32_t test_WADSideDefsSize = 218;
extern const uint32_t test_WADSideDefs[];

constexpr uint32_t test_WADSegmentsSize = 96;
extern const uint32_t test_WADSegments[];

constexpr uint32_t test_WADSubSectorsSize = 7;
extern const uint32_t test_WADSubSectors[];

constexpr uint32_t test_WADSectorsSize = 26;
extern const uint32_t test_WADSectors[];

constexpr uint32_t test_WADNodesSize = 42;
extern const uint32_t test_WADNodes[];

void loadMap_test_WAD(WAD::LevelData& dstLevel);

