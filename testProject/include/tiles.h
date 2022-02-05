#pragma once

#include <cstdint>
#include <cstddef>

#include "vector.h"
#include "Color.h"

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

		uint16_t attribute[4];
	};

	struct alignas(4) AffineTransform
	{
		uint16_t fill0[3];
		int16_t pa;
		uint16_t fill1[3];
		int16_t pb;
		uint16_t fill2[3];
		int16_t pc;
		uint16_t fill3[3];
		int16_t pd;
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
		return reinterpret_cast<STile*>(0x06000000 + n*0x4000);
	}

	static DTile* DTileBlock(uint32_t n)
	{
		return reinterpret_cast<DTile*>(0x06000000 + n*0x4000);
	}

	static Block* OAM()
	{
		return reinterpret_cast<Block*>(0x07000000);
	}
};

inline Color* BackgroundPalette()
{
	return reinterpret_cast<Color*>(0x05000000);
}

inline Color* SpritePalette()
{
	return reinterpret_cast<Color*>(0x05000200);
}