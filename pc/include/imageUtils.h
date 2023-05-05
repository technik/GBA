#pragma once

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include <stb_image.h>
#include <stb_image_write.h>

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

    static Color16b transparent()
    {
        Color16b result;
        result.c = 0;
        return result;
    }

    bool operator==(const Color16b& x) const { return x.c == c; }
    bool operator<(const Color16b& x) const { return c < x.c; }
};

inline auto img_deleter = [](Color4f* ptr)
{
    stbi_image_free(reinterpret_cast<uint8_t*>(ptr));
};

// Raw image
// 32 bpp image as loaded from files.
// Includes 3 color channels + alpha.
// All channel intensities in the range [0,255]
struct RawImage
{
    std::unique_ptr<Color4f[], decltype(img_deleter)> data;
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
        for (int i = 0; i < area(); ++i)
        {
            pixels[i] = Color16b(src.data[i]);
        }
    }

    auto& at(int32_t x, int32_t y)
    {
        return pixels[x + width * y];
    }

    void resize(int32_t x, int32_t y) // Invalidates content
    {
        width = x;
        height = y;
        pixels.resize(area());
    }

    int area() const { return width * height; }
    bool empty() const { return area() == 0; }

    void save(const char* fileName)
    {
        std::vector<Color3f> expandedColor;
        expandedColor.reserve(pixels.size());
        for (auto& c : pixels)
        {
            auto r = c.c & 0x1f;
            auto g = (c.c>>5) & 0x1f;
            auto b = c.c >> 10;
            Color3f& color = expandedColor.emplace_back();
            color.r = r << 3 | r >> 2;
            color.g = g << 3 | g >> 2;
            color.b = b << 3 | b >> 2;
        }
        stbi_write_png(fileName, width, height, 3, expandedColor.data(), width * sizeof(Color3f));
    }
};

struct GreyScaleImage8bit
{
    std::vector<uint8_t> pixels;
    int32_t width = 0;
    int32_t height = 0;

    GreyScaleImage8bit() = default;
    GreyScaleImage8bit(const GreyScaleImage8bit&) = default;
    GreyScaleImage8bit(const RawImage& src)
    {
        resize(src.width, src.height);
        for (int i = 0; i < area(); ++i)
        {
            pixels[i] = src.data[i].r;
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
        width = src.width;
        height = src.height;
        pixels.reserve(area());

        // Prepare the palette
        std::unordered_map<uint16_t, size_t> paletteMap;
        // The first color is reserved for transparency
        palette.push_back(Color16b::transparent());
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

    void breakIntoTiles(std::vector<gfx::DTile>& tiles, std::vector<uint16_t>& tileMap)
    {
        int xTiles = width / 8;
        int yTiles = height / 8;

        auto hasher = [](const gfx::DTile& tile)
        {
            return XXH64(tile.pixel, 64, 0);
        };
        std::unordered_map<gfx::DTile, uint64_t, decltype(hasher)> tileIndex;

        for (int y = 0; y < yTiles; ++y)
        {
            for (int x = 0; x < xTiles; ++x)
            {
                auto tile = getTile(x * 8, y * 8);
                auto iter = tileIndex.find(tile);

                if (iter != tileIndex.end())
                {
                    tileMap.push_back(iter->second);
                }
                else
                {
                    auto nextIndex = tiles.size();
                    tileMap.push_back(nextIndex);
                    tileIndex[tile] = nextIndex;
                    tiles.push_back(tile);
                }
            }
        }
    }

    int area() const { return width * height; }
    bool empty() const { return area() == 0; }
};

inline bool buildPalette(const Image16bit& srcImage, PaletteImage8& dstImage, const std::string& dstFile)
{
    dstImage = PaletteImage8(srcImage);
    const auto& palette = dstImage.palette;

    if (palette.size() > 256)
    {
        std::cout << "Too many unique colors. Image exceeds the 256 color palette\n";
        return false;
    }

    std::cout << "Palette has " << palette.size() << " colors\n";

    // Save palette into an image for debugging
    Image16bit paletteImage;
    paletteImage.resize(16, 16);
    paletteImage.pixels = palette;
    paletteImage.pixels.resize(256, Color16b::transparent()); // Padd the end of the image with transparent color
    paletteImage.save(dstFile.c_str());

    return true;
}

inline void palettize(const RawImage& srcImage, const std::vector<Color16b>& palette, std::vector<uint8_t>& outTiles)
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

                    Color16b c = *p;
                    uint8_t index = std::find(palette.begin(), palette.end(), c) - palette.begin();
                    outTiles.push_back(index); // avoid 0
                }
            }
        }
    }
}