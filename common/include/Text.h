#pragma once

#include <gfx/sprite.h>

class TextSystem
{
public:
    void Init();
	void writeNumbers(const uint8_t* str, Sprite::Object* dst);

private:
    uint32_t mTileStart;
    uint32_t mPaletteStart;
};