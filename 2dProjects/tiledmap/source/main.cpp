// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
#include <base.h>

// Engine code
#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Draw.h>
#include <Keypad.h>
#include <linearMath.h>
#include <noise.h>
#include <Text.h>
#include <Timer.h>
#include <gfx/palette.h>
#include <gfx/sprite.h>
#include <gfx/tile.h>
#include <tools/frameCounter.h>

using namespace math;
using namespace gfx;

class SpriteLinearAllocator
{
public:
	SpriteLinearAllocator(uint8_t capacity)
		: m_Capacity(capacity)
	{
		m_Start = Sprite::ObjectAllocator::alloc(m_Capacity);
		m_Next = 0;
	}

	void reset()
	{
		m_Next = 0;
	}

	// Allocate Sprite Objects in OAM memory
	Sprite::Object* alloc(uint32_t n)
	{
		if(m_Next+n > m_Capacity)
		{
			return nullptr; // Out of memory.
		}
		auto pos = m_Next;
		m_Next += n;
		return &m_Start[pos];
	}

private:

	Sprite::Object* m_Start = 0;
	uint8_t m_Next = 0;
	uint8_t m_Capacity;
};

struct Billboard
{
	Billboard()
	{
		// Alloc tiles
		auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::LowSpriteBank);
		constexpr auto spriteShape = Sprite::Shape::square16x16;
		constexpr auto numTiles = Sprite::GetNumTiles(spriteShape);
		m_tileNdx = tileBank.allocDTiles(numTiles);

		// Init shadow sprite
		m_sprite.Configure(Sprite::ObjectMode::Normal, Sprite::GfxMode::Normal, Sprite::ColorMode::Palette256, spriteShape);
		m_sprite.SetNonAffineTransform(false, false, spriteShape);
		m_sprite.setDTiles(m_tileNdx);
		m_sprite.setPos(120-8, 80-8);

		// Draw into the tiles
		for(uint32_t t = 0; t < numTiles; ++t)
		{
			auto& tile = tileBank.GetDTile(m_tileNdx + t);
			tile.fill(30); // Mid gray
		}
	}

	void Render(SpriteLinearAllocator& spriteAlloc)
	{
		auto* dst = spriteAlloc.alloc(1);
		if(dst)
		{
			memcpy(dst, &m_sprite, sizeof(Sprite::Object));
		}
	}

	bool m_visible = true;
	Sprite::Object m_sprite;
	uint32_t m_tileNdx = 0;
};

template<typename Palette>
void GenerateBasicPalette()
{
	// The first 16 colors store full bright primary colors
	auto primaries = Palette::Allocator::alloc(15);
	// Some basic colors
	Palette::color(primaries + 0) = BasicColor::Red;
	Palette::color(primaries + 1) = BasicColor::Green;
	Palette::color(primaries + 2) = BasicColor::Blue;
	Palette::color(primaries + 3) = BasicColor::Orange;
	Palette::color(primaries + 4) = BasicColor::Yellow;
	Palette::color(primaries + 5) = BasicColor::Purple;
	Palette::color(primaries + 6) = BasicColor::Pink;
	Palette::color(primaries + 7) = BasicColor::LightGrey;
	Palette::color(primaries + 8) = BasicColor::SkyBlue;
	// Setup a greyscale
	auto greyScale = Palette::Allocator::alloc(32);
	for(auto i = 0; i < 32; ++i)
	{
		Palette::color(greyScale + i) = Color(i,i,i); 
	}
}

void InitGraphics()
{
	// Init palettes
	GenerateBasicPalette<gfx::BackgroundPalette>();
	GenerateBasicPalette<gfx::SpritePalette>();

	// Set up mode 0, 256x256 tiles, 256 color palette
	Display().SetMode<0, DisplayControl::BG0>();
	Display().setupBackground(0,0,8,DisplayControl::TiledBGSize::e256x256);
}

void loadMapTileSet()
{
	auto& bgTiles = gfx::TileBank::GetBank(0);
	bgTiles.GetDTile(0).fill(2); // Fill tile 0 with green
	bgTiles.GetDTile(1).fill(3); // Fill tile 1 with blue
}

void loadMapData()
{
	auto bgMap = (uint16_t*)gfx::TileBank::GetBank(1).memory();
	for(int i = 0; i < 8; ++i)
	{
		auto x = 2*i + 32*i;
		bgMap[x] = 1;
	}
}

void cleanSprites()
{
	Sprite::Object nullSprite;
	nullSprite.hide();
	for(int i = 0; i < 128; ++i)
	{
		auto dst = reinterpret_cast<uint32_t*>(&Sprite::OAM_Objects()[i]);
		memcpy(dst, &nullSprite, sizeof(Sprite::Object));
	}
}

void loadPlayerSprite()
{
	//
}

int main()
{
	irq_init(NULL);
	// and vblank int for vsync
	irq_add(II_VBLANK, NULL);
	// Full resolution, paletized color mode.
	Display().StartBlank();

	InitGraphics();
	loadMapTileSet();
	loadMapData();
	cleanSprites();
	Display().enableSprites();
	auto player = Billboard();
	SpriteLinearAllocator spriteAlloc(32);

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while (1)
	{
		spriteAlloc.reset();
		
		VBlankIntrWait();

		player.Render(spriteAlloc);
	}
	return 0;
}
