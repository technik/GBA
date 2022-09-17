#include <cassert>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <xxhash/xxh3.h>

#include <gfx/tile.h>
#include <imageUtils.h>

void serializeData(const void* data, size_t byteCount, const std::string& variableName, std::ostream& out)
{
    // Only multiples of 4 bytes supported
    assert(byteCount % 4 == 0);
    auto dwordCount = byteCount / 4;
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
    assert(byteCount % 4 == 0);
    auto dwordCount = byteCount / 4;

    extern const uint32_t fontTileDataSize;
    extern const uint32_t fontTileData[];

    header << "constexpr uint32_t " << variableName << "Size = " << dwordCount << ";\n";
    header << "extern const uint32_t " << variableName << "[];\n\n";

    serializeData(data, byteCount, variableName, cpp);
}

void serializeWAD(const std::vector<uint8_t>& rawData, const std::string& inputFileName)
{
    // --- Serialize data ---
    std::ofstream outHeader(inputFileName + ".h");
    outHeader << "#pragma once\n#include <cstdint>\n\n";
    std::ofstream outCppFile(inputFileName + ".cpp");
    outCppFile << "#include \"" << inputFileName << ".h\"\n\n";

    // Generate filenames
    std::filesystem::path inputFile = inputFileName;
    auto fileWithoutExtension = inputFile.stem().string();
    auto variableName = fileWithoutExtension + "_WAD";


    appendBuffer(outCppFile, outHeader, variableName, rawData.data(), rawData.size());
    outCppFile << "\n";
}

std::vector<uint8_t> loadRawWAD(std::string_view fileName)
{
    std::vector<uint8_t> rawData;

    std::ifstream wadFile(fileName.data(), std::ios_base::ate | std::ios_base::binary);
    uint32_t size = wadFile.tellg();
    wadFile.seekg(std::ios_base::beg);
    size -= wadFile.tellg();

    rawData.resize(size);

    wadFile.read((char*)rawData.data(), size);

    return rawData;
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
    std::vector<uint8_t> rawWAD = loadRawWAD(fileName);

    if (rawWAD.empty())
    {
        std::cout << "Unable to load WAD file " << fileName << "\n";
        return -1;
    }

    // Write into a header/cpp pair
    serializeWAD(rawWAD, fileName);

    return 0;
}