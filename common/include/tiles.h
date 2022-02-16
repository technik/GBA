#pragma once

#include <cstdint>
#include <cstddef>

#include "vector.h"
#include "Color.h"
#include "Device.h"

union ObjectAttributeMemory
{
	volatile Sprite::Object object[1024];
	volatile Sprite::Transform transform[32];
};

inline ObjectAttributeMemory& OAM()
{
	return *reinterpret_cast<ObjectAttributeMemory*>(OAMAddress);
}

inline auto* BackgroundPalette()
{
	return reinterpret_cast<volatile Color*>(PaletteMemAddress);
}

inline auto* SpritePalette()
{
	return reinterpret_cast<volatile Color*>(PaletteMemAddress + 0x200);
}