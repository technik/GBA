#include "Text.h"

// Import font data
extern const uint32_t fontTileDataSize;
extern const uint32_t fontTileData[];

void TextSystem::Init()
{
    mPaletteStart = SpritePaletteAllocator::alloc(2); // Bg and text colors
    mTileStart = SpriteTileAllocator::alloc(64);

    // Init palette
    SpritePalette()[mPaletteStart+0] = BasicColor::Black;
    SpritePalette()[mPaletteStart+1] = BasicColor::White;

    // Copy the font tiles
    uint32_t colorOffset = (mPaletteStart<<24) | (mPaletteStart<<16) | (mPaletteStart<<8) | mPaletteStart;

    auto* spriteBase = &((volatile uint32_t*)Sprite::DTileBlock(5))[mTileStart];
    for(uint32_t i = 0; i < fontTileDataSize; ++i)
    {
        spriteBase[i] = fontTileData[i];// + colorOffset;
    }
}