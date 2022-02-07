#pragma once

#include <cstdint>
#include <cstddef>

#include "vector.h"
#include "Color.h"
#include "Device.h"

struct Sprite
{
	enum Shape
	{
		Square = 0,
		Wide = 1,
		Tall = 2
	};

	struct alignas(4) ObjectAttribute
	{
		ObjectAttribute() = default;
		ObjectAttribute(math::Vec2i pos, uint32_t shape, uint32_t size);

		volatile uint16_t attribute[4];
	};

	struct alignas(4) AffineTransform
	{
		uint16_t fill0[3];
		volatile int16_t pa;
		uint16_t fill1[3];
		volatile int16_t pb;
		uint16_t fill2[3];
		volatile int16_t pc;
		uint16_t fill3[3];
		volatile int16_t pd;
	};

	union Block
	{
		ObjectAttribute objects[4];
		AffineTransform transform;
	};

	struct STile
	{
		uint8_t pixelPair[32];

		void fill(uint8_t ndx)
		{
			for(int i = 0; i < 32; ++i)
			{
				pixelPair[i] = (ndx|(ndx<<4));
			}
		}

		static uint32_t LowSpriteBankIndex(uint32_t n)
		{
			return n;
		}

		static uint32_t HighSpriteBankIndex(uint32_t n)
		{
			return 512 + n;
		}
	};

	struct DTile
	{
		uint8_t pixel[64];

		void fill(uint8_t ndx)
		{
			for(int i = 0; i < 64; ++i)
			{
				pixel[i] = ndx;
			}
		}

		static uint32_t LowSpriteBankIndex(uint32_t n)
		{
			return 2*n;
		}

		static uint32_t HighSpriteBankIndex(uint32_t n)
		{
			return 512 + 2*n;
		}
	};

	static STile* STileBlock(uint32_t n)
	{
		return reinterpret_cast<STile*>(VideoMemAddress + n*0x4000);
	}

	static DTile* DTileBlock(uint32_t n)
	{
		return reinterpret_cast<DTile*>(VideoMemAddress + n*0x4000);
	}

	static Block* OAM()
	{
		return reinterpret_cast<Block*>(OAMAddress);
	}

	template<size_t oamBank>
	static void HideAllObjects()
	{
		for(int i = 0; i < 32; ++i)
		{
			
		}
	}
};

struct SpritePaletteAllocator
{
	static void reset()
	{
		sEnd = 1; // Start at 1 because 0 means transparent
	}

	static uint32_t  alloc(uint32_t size)
	{
		if(size + sEnd >= MaxNumColors)
		{
			return 0; // Out of memory.
		}
		auto pos = sEnd;
		sEnd += size;
		return pos;
	}

	static constexpr uint32_t MaxNumColors = 256;
	inline static uint32_t sEnd = 1;
};

// Allocate DTiles from the sprite top bank
struct SpriteTileAllocator
{
	static void reset()
	{
		sEnd = 0;
	}

	static uint32_t  alloc(uint32_t size)
	{
		if(size + sEnd >= MaxNumTiles)
		{
			return 0; // Out of memory.
		}
		auto pos = sEnd;
		sEnd += size;
		return pos;
	}

	static constexpr uint32_t MaxNumTiles = 512;
	inline static uint32_t sEnd = 0;
};

union ObjectAttributeMemory
{
	volatile Sprite::ObjectAttribute object[1024];
	volatile Sprite::AffineTransform transform[32];
};

inline ObjectAttributeMemory& OAM()
{
	return *reinterpret_cast<ObjectAttributeMemory*>(OAMAddress);
}

inline Color* BackgroundPalette()
{
	return reinterpret_cast<Color*>(PaletteMemAddress);
}

inline Color* SpritePalette()
{
	return reinterpret_cast<Color*>(PaletteMemAddress + 0x200);
}