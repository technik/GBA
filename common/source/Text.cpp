#include "Text.h"
#include <gfx/palette.h>

// Import font data
extern const uint32_t fontTileDataSize;
extern const uint32_t fontTileData[];

using namespace gfx;

void TextSystem::Init()
{
    mPaletteStart = SpritePalette::Allocator::alloc(2); // Bg and text colors
    mTileStart = SpriteTileAllocator::alloc(SpriteTileAllocator::Bank::High, 64);

    // Init palette
    SpritePalette()[mPaletteStart+0].raw = BasicColor::Black.raw;
    SpritePalette()[mPaletteStart+1].raw = BasicColor::White.raw;

    // Copy the font tiles
    uint32_t colorOffset = (mPaletteStart<<24) | (mPaletteStart<<16) | (mPaletteStart<<8) | mPaletteStart;

    auto* spriteBase = &((volatile uint32_t*)Sprite::DTileBlock(5))[mTileStart];
    for(uint32_t i = 0; i < fontTileDataSize; ++i)
    {
        spriteBase[i] = fontTileData[i];// + colorOffset;
    }
}