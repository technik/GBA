#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <xxhash/xxh3.h>
#include <WAD.h>

#include <gfx/tile.h>
#include <imageUtils.h>

struct WADMetrics
{
    int numVertices;
    int numLineDefs;
    int numSegments;
    int numSectors;
    int numSubsectors;
};

void serializeData(const void* data, size_t byteCount, const std::string& variableName, std::ostream& out)
{
    // Only multiples of 4 bytes supported
    //assert(byteCount % 4 == 0);
    auto dwordCount = (byteCount+3) / 4;
    uint32_t* packedData = (uint32_t*)data;

    out << "extern const uint32_t " << variableName << "[" << dwordCount << "] = {\n";
 
    // Print full rows of data
    while ((dwordCount / 8) > 0)
    {
        out << packedData[0] << ", ";
        out << packedData[1] << ", ";
        out << packedData[2] << ", ";
        out << packedData[3] << ", ";
        out << packedData[4] << ", ";
        out << packedData[5] << ", ";
        out << packedData[6] << ", ";
        out << packedData[7];
        dwordCount -= 8;
        packedData += 8;
        if (dwordCount > 0)
            out << ",\n";
    }
    while (dwordCount > 1)
    {
        out << *packedData << ", ";
        --dwordCount;
        ++packedData;
    }
    if (dwordCount)
        out << *packedData;
    out << "};\n";
}

void appendBuffer(std::ostream& cpp, std::ostream& header, const std::string& variableName, const void* data, std::size_t byteCount)
{
    // Only multiples of 4 bytes supported
    //assert(byteCount % 4 == 0);
    auto dwordCount = (byteCount+3) / 4;

    header << "constexpr uint32_t " << variableName << "Size = " << dwordCount << ";\n";
    header << "extern const uint32_t " << variableName << "[];\n\n";

    serializeData(data, byteCount, variableName, cpp);
}

void serializeWAD(const WAD::LevelData& level, const WADMetrics& metrics, const std::string& inputFileName)
{
    // --- Serialize data ---
    std::ofstream outHeader(inputFileName + ".h");
    outHeader << "#pragma once\n#include <cstdint>\n#include <base.h>\n#include <WAD.h>\n\n";
    std::ofstream outCppFile(inputFileName + ".cpp");
    outCppFile << "#include \"" << inputFileName << ".h\"\n\n";

    // Generate filenames
    std::filesystem::path inputFile = inputFileName;
    auto fileWithoutExtension = inputFile.stem().string();
    auto variableName = fileWithoutExtension + "_WAD";

    appendBuffer(outCppFile, outHeader, variableName + "Vertices", level.vertices, sizeof(WAD::Vertex) * metrics.numVertices);
    appendBuffer(outCppFile, outHeader, variableName + "LineDefs", level.linedefs, sizeof(WAD::LineDef) * metrics.numLineDefs);
    appendBuffer(outCppFile, outHeader, variableName + "Segments", level.segments, sizeof(WAD::Seg) * metrics.numSegments);
    appendBuffer(outCppFile, outHeader, variableName + "Subsectors", level.subsectors, sizeof(WAD::SubSector) * metrics.numSubsectors);
    appendBuffer(outCppFile, outHeader, variableName + "Sectors", level.sectors, sizeof(WAD::Sector) * metrics.numSectors);
    appendBuffer(outCppFile, outHeader, variableName + "Nodes", level.nodes, sizeof(WAD::Node) * level.numNodes);
    outCppFile << "\n";
}

std::vector<uint8_t> loadRawWAD(std::string_view fileName)
{
    std::vector<uint8_t> rawData;

    std::ifstream wadFile(fileName.data(), std::ios_base::ate | std::ios_base::binary);
    uint32_t size = wadFile.tellg();
    wadFile.seekg(std::ios_base::beg);
    size -= wadFile.tellg();

    uint32_t alignedSize = 4 * ((size + 3) / 4);
    assert(alignedSize >= size);
    rawData.resize(alignedSize, 0);

    wadFile.read((char*)rawData.data(), size);

    return rawData;
}

bool loadWAD(WAD::LevelData& dstLevel, WADMetrics& metrics, std::vector<uint8_t>& rawWAD, std::string_view fileName)
{
    rawWAD = loadRawWAD(fileName);
    if (rawWAD.empty())
        return false;

    const uint8_t* byteData = reinterpret_cast<const uint8_t*>(rawWAD.data());
    // Verify WAD format
    const char* defString = reinterpret_cast<const char*>(byteData);
    //assert(!strncmp(defString, "PWAD", 4));
    if (strncmp(defString, "PWAD", 4) != 0)
        return false;

    auto wadHeader = reinterpret_cast<const WAD::Header*>(byteData);

    auto directory = reinterpret_cast<const WAD::Lump*>(&byteData[wadHeader->dirOffset]);

    WAD::Lump localDir[9];
    // Get a local copy of the directory to avoid alignment issues
    auto dirSize = sizeof(WAD::Lump) * 9;
    for (int i = 0; i < dirSize; ++i)
    {
        ((uint8_t*)localDir)[i] = byteData[wadHeader->dirOffset + i];
    }

    // Lump directories
    //auto& mapName = directory[0];
    //auto& things = directory[1];
    auto& lineDefsLump = localDir[2];
    //auto& sideDefsLump = directory[3];
    auto& verticesLump = localDir[4];
    auto& segLumps = localDir[5];
    auto& ssectorLumps = localDir[6];
    auto& nodeLumps = localDir[7];
    auto& sectorLumps = localDir[8];
    //auto& reject = directory[9];
    //auto& blockMap = directory[10];

    // Load vertex data
    metrics.numVertices = verticesLump.dataSize / sizeof(WAD::Vertex);
    dstLevel.vertices = reinterpret_cast<const WAD::Vertex*>(&byteData[verticesLump.dataOffset]);

    // Load line defs
    metrics.numLineDefs = lineDefsLump.dataSize / sizeof(WAD::LineDef);
    dstLevel.linedefs = (const WAD::LineDef*)(&byteData[lineDefsLump.dataOffset]);

    // Load nodes
    dstLevel.numNodes = nodeLumps.dataSize / sizeof(WAD::Node);
    dstLevel.nodes = (const WAD::Node*)(&byteData[nodeLumps.dataOffset]);

    // Load subsectors
    metrics.numSubsectors = ssectorLumps.dataSize / sizeof(WAD::SubSector);
    dstLevel.subsectors = (const WAD::SubSector*)&byteData[ssectorLumps.dataOffset];

    // Load segments
    metrics.numSegments = segLumps.dataSize / sizeof(WAD::Seg);
    dstLevel.segments = (const WAD::Seg*)(&byteData[segLumps.dataOffset]);

    // Load sectors
    metrics.numSectors = sectorLumps.dataSize / sizeof(WAD::Sector);
    dstLevel.sectors = (const WAD::Sector*)&byteData[sectorLumps.dataOffset];

    return true;
}

int main(int _argc, const char** _argv)
{
    // Parse arguments
    if (_argc < 2)
    {
        std::cout << "Not enough arguments\n";
        return -1;
    }

    std::string fileName = _argv[1];

    // Read WAD file into a buffer
    WAD::LevelData parsedWAD;
    WADMetrics metrics;
    std::vector<uint8_t> rawWAD; // Needs to stay alive until after serialization of all the data
    if (!loadWAD(parsedWAD, metrics, rawWAD, fileName))
    {
        std::cout << "Unable to load WAD file " << fileName << "\n";
        return -1;
    }

    // Write into a header/cpp pair
    serializeWAD(parsedWAD, metrics, fileName);

    return 0;
}