#include <string>
#include <iostream>
#include <vector>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

uint16_t reduceColor(const uint8_t* color)
{
    return (color[0] >> 3) | (color[1] >> 3) << 5 | (color[2] >> 3) << 10;
}

void buildPalette(const uint8_t* rawPixels, int numPixels, std::vector<uint16_t>& palette)
{
    for (int i = 0; i < numPixels; ++i)
    {
        const uint8_t* p = &rawPixels[4 * i];
        if (p[3] == 0) // Transparent pixel, not used in the palette.
            continue;

        uint16_t c = reduceColor(p);
        if (std::find(palette.begin(), palette.end(), c) == palette.end())
        {
            palette.push_back(c);
        }
    }
}

void buildTiles(const uint8_t* rawPixels, int w, int h, const std::vector<uint16_t>& palette, std::vector<uint8_t>& outTiles)
{
    for (int j = 0; j < h; j += 8)
    {
        for (int i = 0; i < w; i += 8)
        {
            auto* tile = &rawPixels[4 * (i + j * w)];
            for (int k = 0; k < 8; ++k)
            {
                for (int l = 0; l < 8; ++l)
                {
                    auto* p = &tile[4 * (l + w * k)];
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
    int w, h, n;
    uint8_t *rawPixels = stbi_load(fileName.c_str(), &w, &h, &n, 4);
    if (!rawPixels)
    {
        std::cout << "Image not found\n";
    }
    if (w & 0x7 != w || h & 0x7 != h)
    {
        std::cout << "Image data is not a multiple of tile size";
        return -1;
    }
    h = std::min(h, 16);

    // Palettize
    std::vector<uint16_t> palette;
    std::vector<uint8_t> tileData;
    buildPalette(rawPixels, w * h, palette);
    if (palette.size() > 255)
    {
        std::cout << "Too many unique colors. Image exceeds the 256 color palette\n";
        return -1;
    }

    buildTiles(rawPixels, w, h, palette, tileData);

    // Write into a header
    std::ofstream ss(fileName + ".cpp");
    ss << "static constexpr uint16_t palette[" << palette.size() << "] = {\n";
    for (int i = 0; i < palette.size() - 1; ++i)
    {
        ss << "\t" << palette[i] << ",\n";
    }

    ss << "\t" << palette.back() << "\n};\n\n";


    ss << "static constexpr uint8_t tileData[" << tileData.size() << "] = {\n";
    int i = 0;
    for (int i = 0; i < tileData.size() - 1; i+=8)
    {
        ss << "\t"
            << (int)tileData[i + 0] << ", "
            << (int)tileData[i + 1] << ", "
            << (int)tileData[i + 2] << ", "
            << (int)tileData[i + 3] << ", "
            << (int)tileData[i + 4] << ", "
            << (int)tileData[i + 5] << ", "
            << (int)tileData[i + 6] << ", "
            << (int)tileData[i + 7] << ",\n";
    }

    ss << "\t"
        << (int)tileData[i + 0] << ", "
        << (int)tileData[i + 1] << ", "
        << (int)tileData[i + 2] << ", "
        << (int)tileData[i + 3] << ", "
        << (int)tileData[i + 4] << ", "
        << (int)tileData[i + 5] << ", "
        << (int)tileData[i + 6] << ", "
        << (int)tileData[i + 7] << "\n};\n\n";

    return 0;
}