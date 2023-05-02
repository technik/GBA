#pragma once

#include <Color.h>
#include <Device.h>
#include <cstring>

namespace gfx
{
	struct STile
	{
		uint8_t pixelPair[32];

		uint32_t row(uint32_t n) const
		{
			return reinterpret_cast<const uint32_t*>(pixelPair)[n];
		}
		
		volatile uint32_t& row(uint32_t n) volatile
		{
			return reinterpret_cast<volatile uint32_t*>(pixelPair)[n];
		}

		void fill(uint8_t ndx)
		{
			for(int i = 0; i < 32; ++i)
			{
				pixelPair[i] = (ndx|(ndx<<4));
			}
		}
		
		void fill(uint8_t ndx) volatile
		{
			uint32_t row = (ndx|(ndx<<4));
			row |= row<<8;
			row |= row<<16;
			auto dst = reinterpret_cast<volatile uint32_t*>(pixelPair);
			for(int i = 0; i < 8; ++i)
			{
				dst[i] = row;
			}
		}
	};

	struct DTile
	{
		uint8_t pixel[64];

		bool operator==(const DTile& other) const
		{
			return memcmp(pixel, other.pixel, sizeof(pixel)) == 0;
		}

		void fill(uint8_t ndx)
		{
			for(int i = 0; i < 64; ++i)
			{
				pixel[i] = ndx;
			}
		}
#pragma GCC push_options
#pragma GCC optimize ("O0")
		void fill(uint8_t ndx) volatile
		{
			for(int i = 0; i < 64; ++i)
			{
				pixel[i] = ndx;
			}
		}
#pragma GCC pop_options

		void borderTile(uint8_t centerColor, uint8_t borderColor) volatile
		{
			for(uint32_t i = 0; i < 8; ++i)
			{
				pixel[i] = borderColor;
				pixel[56+i] = borderColor;
			}
			for(int i = 8; i < 56; ++i)
			{
				auto x = i&0x7;
				pixel[i] = (x==1)||(x==7) ? borderColor : centerColor;
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

	// VRAM contains 6 banks of character data, each of them 16kb in size.
	// You can create a TileBank to allocate tiles from starting at any of
	// those banks, and it will potentially span until the end of the next bank.
	class TileBank
	{
	public:
		TileBank(uint32_t bankIndex);
		void reset();

		// Allocate small tiles where each dot is only 4 bits.
		// Returns the STile index
		uint32_t allocSTiles(uint32_t size);

		// Allocate full tiles where each dot is 8 bits.
		// Returns the DTile index
		uint32_t allocDTiles(uint32_t size);

		volatile STile& GetSTile(uint32_t index);
		volatile DTile& GetDTile(uint32_t index);

		static TileBank& GetBank(uint32_t bankIndex);

		void* memory() { return reinterpret_cast<void*>(mBaseAddress); }

		static constexpr uint32_t LowSpriteBank = 4;
		static constexpr uint32_t HighSpriteBank = 5;

	private:
		uint32_t mBaseAddress;
		uint32_t mNextTile;
	};

}	// namespace gfx