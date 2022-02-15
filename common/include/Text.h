#pragma once

#include <gfx/sprite.h>

class TextSystem
{
public:
    void Init();
	void writeNumbers(const char* str, Sprite::Object* dst);

private:
    uint32_t mTileStart;
    uint32_t mPaletteStartNdx;
};