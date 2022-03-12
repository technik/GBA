#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>
#include <unordered_map>
#include <xxhash/xxh3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <gfx/tile.h>

struct Color3f
{
    uint8_t r, g, b;
};

struct Color4f
{
    uint8_t r, g, b, a;
};

struct Color16b
{
    uint16_t c;

    // Constructors
    Color16b() = default;

    Color16b(const Color16b&) = default;

    Color16b(const Color3f& x)
    {
        c = (x.r >> 3) | ((x.g >> 3) << 5) | ((x.b >> 3) << 10) | (1 << 15); // Most significant bit prevents transparency on black
    }

    Color16b(const Color4f& x)
    {
        c = x.a ? ((x.r >> 3) | ((x.g >> 3) << 5) | ((x.b >> 3) << 10) | (1 << 15)) : 0;
    }
};

auto img_deleter = [](Color4f* ptr)
{
    stbi_image_free(reinterpret_cast<uint8_t*>(ptr));
};
// Raw image
// 32 bpp image as loaded from files.
// Includes 3 color channels + alpha.
// All channel intensities in the range [0,255]
struct RawImage
{
     std::unique_ptr<Color4f[],decltype(img_deleter)> data;
     int32_t width = 0;
     int32_t height = 0;

     int area() const { return width * height; }
     bool empty() const { return area() == 0; }

     bool load(const char* fileName)
     {
         int nChannels;
         uint8_t* rawPixels = stbi_load(fileName, &width, &height, &nChannels, 4);
         if (!rawPixels)
         {
             false;
         }
         data = std::unique_ptr<Color4f[], decltype(img_deleter)>(reinterpret_cast<Color4f*>(rawPixels), img_deleter);
         return true;
     }
};

struct TiledImage
{

};

struct Image16bit
{
    std::vector<Color16b> pixels;
    int32_t width = 0;
    int32_t height = 0;

    Image16bit() = default;
    Image16bit(const Image16bit&) = default;
    Image16bit(const RawImage& src)
    {
        resize(src.width, src.height);
        for(int i = 0; i < area(); ++i)
        {
            pixels[i] = Color16b(src.data[i]);
        }
    }

    void resize(int32_t x, int32_t y) // Invalidates content
    {
        width = x;
        height = y;
        pixels.resize(area());
    }

    int area() const { return width * height; }
    bool empty() const { return area() == 0; }
};

// Palettized image (4b, 8b)
struct PaletteImage8
{
    std::vector<Color16b> palette;

    std::vector<uint8_t> pixels;
    int32_t width = 0;
    int32_t height = 0;

    PaletteImage8() = default;
    PaletteImage8(const PaletteImage8&) = default;
    PaletteImage8(const Image16bit& src)
    {
        width = src.width/8;
        height = src.height/8;
        pixels.reserve(area());

        // Prepare the palette
        std::unordered_map<uint16_t, size_t> paletteMap;
        // The first color is reserved for transparency
        palette.push_back(Color16b({0,0,0,0}));
        paletteMap[0] = 0;

        // Translate pixels as we build the palette
        for (int i = 0; i < area(); ++i)
        {
            auto color = src.pixels[i].c;
            
            auto iter = paletteMap.find(color);
            if (iter == paletteMap.end())
            {
                paletteMap[color] = palette.size();
                pixels.push_back(palette.size());
                palette.push_back(src.pixels[i]);
            }
            else
            {
                pixels.push_back(iter->second);
            }
        }

        // Enforce even sized palette
        if (palette.size() % 2 != 0)
            palette.push_back(Color16b({ 0, 0, 0, 0 }));
    }

    void resize(int32_t x, int32_t y) // Invalidates content
    {
        width = x;
        height = y;
        pixels.resize(area());
    }

    gfx::DTile getTile(int x0, int y0)
    {
        gfx::DTile result;
        for (int row = 0; row < 8; ++row)
        {
            const auto* rowStart = &pixels[(y0 + row) * width + x0];
            for (int col = 0; col < 8; ++col)
            {
                result.pixel[col + 8 * row] = rowStart[col];
            }
        }

        return result;
    }

    void saveIndexImage(const char* fileName) const
    {
        stbi_write_png(fileName, width, height, 1, pixels.data(), width);
    }

    int area() const { return width * height; }
    bool empty() const { return area() == 0; }
};

// Color15Bit
// STile, DTile
// Colorf

uint16_t reduceColor(const uint8_t* color)
{
    return (color[0] >> 3) | (color[1] >> 3) << 5 | (color[2] >> 3) << 10;
}

void buildPalette(const RawImage& srcImage, std::vector<uint16_t>& palette)
{
    int numPixels = srcImage.area();

    for (int i = 0; i < numPixels; ++i)
    {
        const auto* p = &srcImage.data[4 * i];
        if (p->a == 0) // Transparent pixel, not used in the palette.
            continue;

        Color16b c = *p;
        if (std::find(palette.begin(), palette.end(), c.c) == palette.end())
        {
            palette.push_back(c.c);
        }
    }
}

void buildTiles(const RawImage& srcImage, const std::vector<uint16_t>& palette, std::vector<uint8_t>& outTiles)
{
    for (int j = 0; j < srcImage.height; j += 8)
    {
        for (int i = 0; i < srcImage.width; i += 8)
        {
            auto* tile = &srcImage.data[4 * (i + j * srcImage.width)];
            for (int k = 0; k < 8; ++k)
            {
                for (int l = 0; l < 8; ++l)
                {
                    auto* p = &tile[4 * (l + srcImage.width * k)];
                    if (p->a == 0) // Transparent pixel, not used in the palette.
                    {
                        outTiles.push_back(0); // Transparent pixel;
                    }

                    auto c = reduceColor(&p->r);
                    uint8_t index = std::find(palette.begin(), palette.end(), c) - palette.begin();
                    outTiles.push_back(index+1); // avoid 0
                }
            }
        }
    }
}

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