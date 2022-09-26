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

struct WADMetrics
{
    int numVertices;
    int numLineDefs;
    int numSideDefs;
    int numSegments;
    int numSectors;
    int numSubsectors;
    int numNodes;

    // BBox
    int minX, minY;
    int maxX, maxY;

    int totalSize;

    void print()
    {
        std::cout << "BBox: (" << minX << "," << minY << ") to (" << maxX << "," << maxY << ")\n";
        std::cout << "Vertices: " << numVertices << "\n";
        std::cout << "Lines: " << numLineDefs << "\n";
        std::cout << "Sides: " << numSideDefs << "\n";
        std::cout << "Segments: " << numSegments << "\n";
        std::cout << "Sectors: " << numSectors << "\n";
        std::cout << "SubSectors: " << numSubsectors << "\n";
        std::cout << "BSP Nodes: " << numNodes << "\n";
        std::cout << "Total size: " << totalSize << "\n";
    }
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

void writeLoadFunction(std::ofstream& header, std::ofstream& cpp, std::string mapName)
{
    // Write the prototype
    header << "void loadMap_" << mapName << "(WAD::LevelData& dstLevel);\n\n";

    // Write the implementation
    cpp << "void loadMap_" << mapName << "(WAD::LevelData& dstLevel) {\n"
        << "\t// Load vertex data\n"
        << "\tdstLevel.vertices = (const WAD::Vertex*)" << mapName << "Vertices;\n"
        << "\n"
        << "\t// Load line defs\n"
        << "\tdstLevel.lineDefs = (const WAD::LineDef*)" << mapName << "LineDefs;\n"
        << "\n"
        << "\t// Load side defs\n"
        << "\tdstLevel.sideDefs = (const WAD::SideDef*)" << mapName << "SideDefs;\n"
        << "\n"
        << "\t// Load subsectors defs\n"
        << "\tdstLevel.subSectors = (const WAD::SubSector*)" << mapName << "SubSectors;\n"
        << "\n"
        << "\t// Load segments defs\n"
        << "\tdstLevel.segments = (const WAD::Seg*)" << mapName << "Segments;\n"
        << "\n"
        << "\t// Load sectors defs\n"
        << "\tdstLevel.sectors = (const WAD::Sector*)" << mapName << "Sectors;\n"
        << "\n"
        << "\t// Load nodes\n"
        << "\tdstLevel.numNodes = (" << mapName << "NodesSize * 4) / sizeof(WAD::Node);\n"
        << "\tdstLevel.nodes = (const WAD::Node*)" << mapName << "Nodes;\n"
        << "}\n";
}

void adjustUnits(const WAD::LevelData& level, WADMetrics& metrics)
{
    // Vertices
    if (!metrics.numVertices)
        return;

    auto vertices = const_cast<WAD::Vertex*>(level.vertices);

    // Compute bounding box and center the level around 0 for better use of our finite range
    int minX = vertices[0].x.raw;
    int minY = vertices[0].y.raw;
    int maxX = vertices[0].x.raw;
    int maxY = vertices[0].y.raw;
    for (int i = 0; i < metrics.numVertices; ++i)
    {
        int x = vertices[i].x.raw;
        int y = vertices[i].y.raw;
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
    }
    int x0 = (minX + maxX) / 2;
    int y0 = (minY + maxY) / 2;
    metrics.minX = minX - x0;
    metrics.minY = minY - y0;
    metrics.maxX = maxX - x0;
    metrics.maxY = maxY - y0;

    for (int i = 0; i < metrics.numVertices; ++i)
    {
        vertices[i].x.raw = (vertices[i].x.raw - x0) * 8;
        vertices[i].y.raw = (vertices[i].y.raw - y0) * 8;
    }

    // Nodes
    auto nodes = const_cast<WAD::Node*>(level.nodes);
    for (int i = 0; i < metrics.numNodes; ++i)
    {
        nodes[i].x.raw = (nodes[i].x.raw - x0) * 8;
        nodes[i].y.raw = (nodes[i].y.raw - y0) * 8;
        nodes[i].dx.raw = nodes[i].x.raw * 8;
        nodes[i].dy.raw = nodes[i].y.raw * 8;
    }

    // TODO: AABBs and possibly texture offsets

    // Sector heights
    auto sectors = const_cast<WAD::Sector*>(level.sectors);
    for (int i = 0; i < metrics.numSectors; ++i)
    {
        sectors[i].ceilingHeight.raw *= 10;
    }
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
    appendBuffer(outCppFile, outHeader, variableName + "LineDefs", level.lineDefs, sizeof(WAD::LineDef) * metrics.numLineDefs);
    appendBuffer(outCppFile, outHeader, variableName + "SideDefs", level.sideDefs, sizeof(WAD::SideDef) * metrics.numSideDefs);
    appendBuffer(outCppFile, outHeader, variableName + "Segments", level.segments, sizeof(WAD::Seg) * metrics.numSegments);
    appendBuffer(outCppFile, outHeader, variableName + "SubSectors", level.subSectors, sizeof(WAD::SubSector) * metrics.numSubsectors);
    appendBuffer(outCppFile, outHeader, variableName + "Sectors", level.sectors, sizeof(WAD::Sector) * metrics.numSectors);
    appendBuffer(outCppFile, outHeader, variableName + "Nodes", level.nodes, sizeof(WAD::Node) * level.numNodes);
    outCppFile << "\n";
    writeLoadFunction(outHeader, outCppFile, variableName);
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
    auto& sideDefsLump = directory[3];
    auto& verticesLump = localDir[4];
    auto& segLumps = localDir[5];
    auto& ssectorLumps = localDir[6];
    auto& nodeLumps = localDir[7];
    auto& sectorLumps = localDir[8];
    //auto& reject = directory[9];
    //auto& blockMap = directory[10];

    metrics.totalSize =
        lineDefsLump.dataSize +
        sideDefsLump.dataSize +
        verticesLump.dataSize +
        segLumps.dataSize +
        ssectorLumps.dataSize +
        nodeLumps.dataSize +
        sectorLumps.dataSize;

    // Load vertex data
    metrics.numVertices = verticesLump.dataSize / sizeof(WAD::Vertex);
    dstLevel.vertices = reinterpret_cast<const WAD::Vertex*>(&byteData[verticesLump.dataOffset]);

    // Load line defs
    metrics.numLineDefs = lineDefsLump.dataSize / sizeof(WAD::LineDef);
    dstLevel.lineDefs = (const WAD::LineDef*)(&byteData[lineDefsLump.dataOffset]);

    // Load side defs
    metrics.numSideDefs = sideDefsLump.dataSize / sizeof(WAD::SideDef);
    dstLevel.sideDefs = (const WAD::SideDef*)(&byteData[sideDefsLump.dataOffset]);

    // Load nodes
    dstLevel.numNodes = nodeLumps.dataSize / sizeof(WAD::Node);
    metrics.numNodes = dstLevel.numNodes;
    dstLevel.nodes = (const WAD::Node*)(&byteData[nodeLumps.dataOffset]);

    // Load subsectors
    metrics.numSubsectors = ssectorLumps.dataSize / sizeof(WAD::SubSector);
    dstLevel.subSectors = (const WAD::SubSector*)&byteData[ssectorLumps.dataOffset];

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

    // Translate units from "Doom compatible" to a common frame where we correct for Doom's 1.25 aspect ratio.
    adjustUnits(parsedWAD, metrics);

    // Write into a header/cpp pair
    serializeWAD(parsedWAD, metrics, fileName);

    // Finally print metrics
    metrics.print();

    return 0;
}