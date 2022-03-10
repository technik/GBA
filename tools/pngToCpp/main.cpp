#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

auto img_deleter = [](uint8_t* ptr)
{
    stbi_image_free(ptr);
};
// Raw image
// 32 bpp image as loaded from files.
// Includes 3 color channels + alpha.
// All channel intensities in the range [0,255]
struct RawImage
{
     std::unique_ptr<uint8_t[],decltype(img_deleter)> data;
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
         data = std::unique_ptr<uint8_t[], decltype(img_deleter)>(rawPixels, img_deleter);
         return true;
     }
};

// Palettized image (8b, 16b)
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
        const uint8_t* p = &srcImage.data[4 * i];
        if (p[3] == 0) // Transparent pixel, not used in the palette.
            continue;

        uint16_t c = reduceColor(p);
        if (std::find(palette.begin(), palette.end(), c) == palette.end())
        {
            palette.push_back(c);
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
                    if (p[3] == 0) // Transparent pixel, not used in the palette.
                    {
                        outTiles.push_back(0); // Transparent pixel;
                    }

                    auto c = reduceColor(p);
                    uint8_t index = std::find(palette.begin(), palette.end(), c) - palette.begin();
                    outTiles.push_back(index+1); // avoid 0
                }
            }
        }
    }
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

    // Read PNG into a buffer
    RawImage srcImage;
    if(srcImage.load(fileName.c_str()))
    {
        std::cout << "Image not found\n";
    }
    if (srcImage.width & 0x7 != srcImage.width || srcImage.height & 0x7 != srcImage.height)
    {
        std::cout << "Image data is not a multiple of tile size";
        return -1;
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