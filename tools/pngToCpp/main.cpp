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

enum class ColorFormat
{
    e15bit,
    e256,
    e16
};

struct ProgramOptions
{
    bool buildMapBg = false;
    bool breakdownTiles = false;
    bool palettize = false;
    ColorFormat targetColor = ColorFormat::e256;
    std::string output;

    std::string fileName;

    bool processArguments(int _argc, const char** _argv)
    {
        if (_argc < 2)
        {
            std::cout << "Not enough arguments\n";
            return false;
        }

        fileName = _argv[1];

        for (int i = 2; i < _argc; ++i)
        {
            auto argi = std::string(_argv[i]);
            if (argi == "--bgMap")
            {
                buildMapBg = true;
            }
            else if (argi == "--tileset")
            {
                breakdownTiles = true;
            }
        }

        if(output.empty() && !fileName.empty())
        {
            output = fileName.substr(0, fileName.find_last_of('.'));
        }

        return true;
    }
};

template<class T>
void dumpRawData(std::vector<T>& typedData, std::ostream& dst)
{
    std::vector<uint32_t> data;
    data.resize((typedData.size() * sizeof(T) + sizeof(uint32_t) - 1) / sizeof(uint32_t), 0);
    memcpy(data.data(), typedData.data(), typedData.size() * sizeof(T));

    // Pad data
    auto rest = data.size() % 16;
    if (rest > 0 || data.size() == 0)
    {
        auto pad = 16 - rest;
        data.resize(data.size() + pad, 0);
    }

    int i = 0;
    for (int i = 0; i < data.size() - 16; i += 16)
    {
        dst << "\t"
            << (uint32_t)data[i + 0] << ", "
            << (uint32_t)data[i + 1] << ", "
            << (uint32_t)data[i + 2] << ", "
            << (uint32_t)data[i + 3] << ", "
            << (uint32_t)data[i + 4] << ", "
            << (uint32_t)data[i + 5] << ", "
            << (uint32_t)data[i + 6] << ", "
            << (uint32_t)data[i + 7] << ", "
            << (uint32_t)data[i + 8] << ", "
            << (uint32_t)data[i + 9] << ", "
            << (uint32_t)data[i + 10] << ", "
            << (uint32_t)data[i + 11] << ", "
            << (uint32_t)data[i + 12] << ", "
            << (uint32_t)data[i + 13] << ", "
            << (uint32_t)data[i + 14] << ", "
            << (uint32_t)data[i + 15] << ",\n";
    }

    dst << "\t"
        << (uint32_t)data[i + 0] << ", "
        << (uint32_t)data[i + 1] << ", "
        << (uint32_t)data[i + 2] << ", "
        << (uint32_t)data[i + 3] << ", "
        << (uint32_t)data[i + 4] << ", "
        << (uint32_t)data[i + 5] << ", "
        << (uint32_t)data[i + 6] << ", "
        << (uint32_t)data[i + 7] << ", "
        << (uint32_t)data[i + 8] << ", "
        << (uint32_t)data[i + 9] << ", "
        << (uint32_t)data[i + 10] << ", "
        << (uint32_t)data[i + 11] << ", "
        << (uint32_t)data[i + 12] << ", "
        << (uint32_t)data[i + 13] << ", "
        << (uint32_t)data[i + 14] << ", "
        << (uint32_t)data[i + 15] << ",\n};\n\n";
}

int main(int _argc, const char** _argv)
{
    // Parse arguments
    ProgramOptions programOptions;
    if (!programOptions.processArguments(_argc, _argv))
    {
        return -1;
    }

    // Read PNG into a buffer
    RawImage srcImage;
    if(!srcImage.load(programOptions.fileName.c_str()))
    {
        std::cout << "Image not found\n";
        return -1;
    }
    if (srcImage.width & 0x7 != srcImage.width || srcImage.height & 0x7 != srcImage.height)
    {
        std::cout << "Image data is not a multiple of tile size";
        return -1;
    }

    // Legacy option for affine backgrounds
    if (programOptions.buildMapBg)
    {
        buildMapAffineBackground(srcImage, programOptions.fileName);
        return 0;
    }

    // Discretize colors
    Image16bit image16 = Image16bit(srcImage);

    // Palettize
    PaletteImage8 palettizedImage;
    if (!buildPalette(image16, palettizedImage, programOptions.output + "-palette.png"))
    {
        return -1;
    }
    std::vector<Color16b>& palette = palettizedImage.palette;

    // Break image into tiles
    std::vector<gfx::DTile> tiles;
    std::vector<uint16_t> tileMap;
    palettizedImage.breakIntoTiles(tiles, tileMap);
    
    // Write into a header
    std::string tag = programOptions.output;
    std::ofstream ss(tag + ".cpp");
    ss << "#include <cstdint>\n\n";
    size_t palette32Size = palette.size() / 2;
    ss << "extern const uint32_t " << tag << "Palette[" << palette32Size << "] = {\n";
    auto* palette32 = reinterpret_cast<const uint32_t*>(palette.data());
    for (int i = 0; i < palette32Size - 1; ++i)
    {
        ss << "\t" << palette32[i] << ",\n";
    }

    ss << "\t" << palette32[palette32Size-1] << "\n};\n\n";


    size_t tile32Size = tiles.size() * sizeof(gfx::DTile) / 4;
    ss << "extern const uint32_t " << tag << "TileSize = " << tile32Size << ";\n";

    ss << "extern const uint32_t " << tag << "TileData[" << tile32Size << "] = {\n";    
    dumpRawData(tiles, ss);

    tile32Size = tileMap.size() * sizeof(uint16_t) / 4;
    ss << "extern const uint32_t " << tag << "TileMapSize = " << tile32Size << ";\n";

    ss << "extern const uint32_t " << tag << "TileMapData[" << tile32Size << "] = {\n";
    dumpRawData(tileMap, ss);

    ss << "}; \n\n";

    return 0;
}