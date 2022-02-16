#include "Text.h"
#include <gfx/palette.h>
#include <gfx/tile.h>
#include <gfx/sprite.h>

// Import font data
extern const uint32_t fontTileDataSize;
extern const uint32_t fontTileData[];

using namespace gfx;

void TextSystem::Init()
{
    // Init palette
    mPaletteStart = SpritePalette::Allocator::alloc(2); // Bg and text colors
    SpritePalette::color(mPaletteStart+0).raw = BasicColor::Black.raw;
    SpritePalette::color(mPaletteStart+1).raw = BasicColor::White.raw;

    // Init tiles
    auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::HighSpriteBank);
    mTileStart = tileBank.allocSTiles(64);

    // Copy the font tiles
    uint32_t colorOffset = (mPaletteStart<<24) | (mPaletteStart<<16) | (mPaletteStart<<8) | mPaletteStart;

    auto* spriteBase = &reinterpret_cast<volatile uint32_t*>(&tileBank.GetSTile(mTileStart))[mTileStart];
    for(uint32_t i = 0; i < fontTileDataSize; ++i)
    {
        spriteBase[i] = fontTileData[i]; // + colorOffset;
    }
}