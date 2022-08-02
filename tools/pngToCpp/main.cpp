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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


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

void buildMapAffineBackground(const RawImage& srcImage, const std::string& inputFileName)
{
    // Generate filenames
    std::filesystem::path inputFile = inputFileName;
    auto fileWithoutExtension = inputFile.stem();

    // Build a map of tiles
    int tileCols = srcImage.width / 8;
    int tileRows = srcImage.height / 8;

    // Convert image to 15bit color
    Image16bit map16bit = srcImage;
    // Palettize the image
    PaletteImage8 palettizedMap = map16bit;

    auto hasher = [](const gfx::DTile& tile)
    {
        return XXH64(tile.pixel, 64, 0);
    };

    std::vector<gfx::DTile> mapTiles;
    std::unordered_map<gfx::DTile, uint64_t, decltype(hasher)> tileMap;
    std::vector<uint8_t> tileIndices;

    for (int row = 0; row < tileRows; ++row)
    {
        for (int col = 0; col < tileCols; ++col)
        {
            auto tile = palettizedMap.getTile(8 * col, 8 * row);
            auto iter = tileMap.find(tile);
            if (iter != tileMap.end())
            {
                tileIndices.push_back(iter->second);
            }
            else
            {
                auto nextIndex = mapTiles.size();
                tileIndices.push_back(nextIndex);
                tileMap[tile] = nextIndex;
                mapTiles.push_back(tile);
            }
        }
    }

    // --- Debug output ---
    
    // Export tile map for debug/preview?

    // --- Serialize data ---
    std::ofstream outHeader(fileWithoutExtension.string() + ".h");
    outHeader << "#pragma once\n#include <cstdint>\n\n";
    std::ofstream outCppFile(fileWithoutExtension.string() + ".cpp");
    outCppFile << "#include \"" << fileWithoutExtension.string() << ".h\"\n\n";

    appendBuffer(outCppFile, outHeader, "mapPalette", palettizedMap.palette.data(), palettizedMap.palette.size() * sizeof(Color16b));
    appendBuffer(outCppFile, outHeader, "bgTiles", mapTiles.data(), mapTiles.size() * sizeof(gfx::DTile));
    outCppFile << "\n";
    appendBuffer(outCppFile, outHeader, "mapData", tileIndices.data(), tileIndices.size());

    // Print stats
    std::cout << "Palette size: " << palettizedMap.palette.size() << "\n";
    std::cout << "Num tiles: " << mapTiles.size() << "\n";
    std::cout << "Map Size: " << palettizedMap.width << "x" << palettizedMap.height << "\n";
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

    bool buildMapBg = false;
    for (int i = 2; i < _argc; ++i)
    {
        if (std::string(_argv[i]) == "--bgMap")
            buildMapBg = true;
    }

    // Read PNG into a buffer
    RawImage srcImage;
    if(!srcImage.load(fileName.c_str()))
    {
        std::cout << "Image not found\n";
        return -1;
    }
    if (srcImage.width & 0x7 != srcImage.width || srcImage.height & 0x7 != srcImage.height)
    {
        std::cout << "Image data is not a multiple of tile size";
        return -1;
    }

    if (buildMapBg)
    {
        buildMapAffineBackground(srcImage, fileName);
        return 0;
    }

    // Palettize
    std::vector<uint16_t> palette;
    std::vector<uint8_t> tileData;
    buildPalette(srcImage, palette);
    if (palette.size() > 255)
    {
        std::cout << "Too many unique colors. Image exceeds the 256 color palette\n";
        return -1;
    }

    buildTiles(srcImage, palette, tileData);

    // Write into a header
    std::ofstream ss(fileName + ".cpp");
    ss << "#include <cstdint>\n\n";
    size_t palette32Size = palette.size() / 2;
    ss << "extern const uint32_t fontPalette[" << palette32Size << "] = {\n";
    auto* palette32 = reinterpret_cast<const uint32_t*>(palette.data());
    for (int i = 0; i < palette32Size - 1; ++i)
    {
        ss << "\t" << palette32[i] << ",\n";
    }

    ss << "\t" << palette32[palette32Size-1] << "\n};\n\n";

    size_t tile32Size = tileData.size() / 4;
    ss << "extern const uint32_t fontTileDataSize = " << tile32Size << ";\n";

    auto tile32 = reinterpret_cast<const uint32_t*>(tileData.data());
    ss << "extern const uint32_t fontTileData[" << tile32Size << "] = {\n";
    int i = 0;
    for (int i = 0; i < tile32Size - 16; i+=16)
    {
        ss << "\t"
            << (int)tile32[i + 0] << ", "
            << (int)tile32[i + 1] << ", "
            << (int)tile32[i + 2] << ", "
            << (int)tile32[i + 3] << ", "
            << (int)tile32[i + 4] << ", "
            << (int)tile32[i + 5] << ", "
            << (int)tile32[i + 6] << ", "
            << (int)tile32[i + 7] << ", "
            << (int)tile32[i + 8] << ", "
            << (int)tile32[i + 9] << ", "
            << (int)tile32[i +10] << ", "
            << (int)tile32[i +11] << ", "
            << (int)tile32[i +12] << ", "
            << (int)tile32[i +13] << ", "
            << (int)tile32[i +14] << ", "
            << (int)tile32[i +15] << ",\n";
    }

    ss << "\t"
        << (int)tile32[i + 0] << ", "
        << (int)tile32[i + 1] << ", "
        << (int)tile32[i + 2] << ", "
        << (int)tile32[i + 3] << ", "
        << (int)tile32[i + 4] << ", "
        << (int)tile32[i + 5] << ", "
        << (int)tile32[i + 6] << ", "
        << (int)tile32[i + 7] << ", "
        << (int)tile32[i + 8] << ", "
        << (int)tile32[i + 9] << ", "
        << (int)tile32[i + 10] << ", "
        << (int)tile32[i + 11] << ", "
        << (int)tile32[i + 12] << ", "
        << (int)tile32[i + 13] << ", "
        << (int)tile32[i + 14] << ", "
        << (int)tile32[i + 15] << ",\n};\n\n";

    return 0;
}