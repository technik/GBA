#include <gfx/tile.h>

static gfx::TileBank gTileBanks[6] = 
{
	gfx::TileBank(0),
	gfx::TileBank(1),
	gfx::TileBank(2),
	gfx::TileBank(3),
	gfx::TileBank(4),
	gfx::TileBank(5)
};

namespace gfx
{
	TileBank::TileBank(uint32_t bankIndex)
		: mBaseAddress(VideoMemAddress + 0x4000 * bankIndex)
		, mNextTile(0)
	{}

	void TileBank::reset()
	{
		mNextTile = 0;
	}

	// Allocate small tiles where each dot is only 4 bits.
	// This type of STile splits the color palette into 16 smaller
	// dedicated palettes.
	// Returns the STile index
	uint32_t TileBank::allocSTiles(uint32_t size)
	{
		if(size + mNextTile >= 1024)
		{
			return uint32_t(-1); // Out of memory
		}
		uint32_t pos = mNextTile;
		mNextTile += size;
		return pos;
	}

	// Allocate small tiles where each dot is 8 bits.
	// This type of tile uses a regular palette
	// Returns the DTile index
	uint32_t TileBank::allocDTiles(uint32_t size)
	{
		// Round up to the next even (i.e. DTile aligned) slot
		auto pos0 = (mNextTile+1)>>1;
		if(pos0 + size*2 >= 512)
		{
			return uint32_t(-1); // Out of memory
		}
		mNextTile = 2*(pos0+size);
		return pos0;
	}

	STile& TileBank::GetSTile(uint32_t index)
	{
		return reinterpret_cast<STile*>(mBaseAddress)[index];
	}

	DTile& TileBank::GetDTile(uint32_t index)
	{
		return reinterpret_cast<DTile*>(mBaseAddress)[index];
	}

	TileBank& TileBank::GetBank(uint32_t bankIndex)
	{
		return gTileBanks[bankIndex];
	}
	
}	// namespace gfx