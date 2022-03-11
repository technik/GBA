#pragma once

#include <Color.h>
#include <Device.h>

namespace gfx
{
	// The GBA has two separate palettes: one for sprites and one for backgrounds.
	static constexpr uint32_t BackgroundPaletteAddress = PaletteMemAddress;
	static constexpr uint32_t SpritePaletteAddress = PaletteMemAddress + PaletteMemSize;
	
	template<uint32_t StartAddressAddress>
	struct Palette
	{
		class Allocator
		{
		public:
			static void reset()
			{
				// End is the transparency color, so we can't allocate it from the palette,
				// so reset to 1 instead of 0
				sEnd = 1;
			}

			static uint32_t alloc(uint32_t size)
			{
				if(size + sEnd >= MaxNumColors)
				{
					return 0; // Out of memory, return transparent
				}
				auto pos = sEnd;
				sEnd += size;
				return pos;
			}

		private:
			static constexpr uint32_t MaxNumColors = 256;
			inline static uint32_t sEnd = 1;
		};

		static uint32_t* rawMemory()
		{
			return reinterpret_cast<uint32_t*>(StartAddressAddress);
		}

		static volatile Color& color(uint32_t n)
		{
			return reinterpret_cast<volatile Color*>(StartAddressAddress)[n];
		}
	};

	using SpritePalette = Palette<SpritePaletteAddress>;
	using BackgroundPalette = Palette<BackgroundPaletteAddress>;

}	// namespace gfx