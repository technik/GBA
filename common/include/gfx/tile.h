#pragma once

#include <Color.h>
#include <Device.h>

namespace gfx
{
	struct STile
	{
		uint8_t pixelPair[32];

		uint32_t row(uint32_t n) const
		{
			return reinterpret_cast<const uint32_t*>(pixelPair)[n];
		}
		
		uint32_t& row(uint32_t n)
		{
			return reinterpret_cast<uint32_t*>(pixelPair)[n];
		}

		void fill(uint8_t ndx)
		{
			for(int i = 0; i < 32; ++i)
			{
				pixelPair[i] = (ndx|(ndx<<4));
			}
		}
	};

	struct DTile
	{
		volatile uint8_t pixel[64];

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

		// Allocate small tiles where each dot is only 4 bits.
		// Returns the DTile index
		uint32_t allocDTiles(uint32_t size);

		static TileBank& GetBank(uint32_t bankIndex);

		static constexpr uint32_t LowSpriteBank = 4;
		static constexpr uint32_t HighSpriteBank = 5;
	};

}	// namespace gfx