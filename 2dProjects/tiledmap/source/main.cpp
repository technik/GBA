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

// Map data
#include <tale.h>

uint8_t mapHeight[32*32] = {};

const uint16_t kRiverMap[32*32] = {
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,17,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,214,214,214,214,214,214,214,214,214,214,214,214,214,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,215,215,215,215,215,215,215,215,215,215,215,215,215,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,216,216,216,216,216,216,216,216,216,216,216,216,216,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,17,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,17,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,15,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,17,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,117,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,19,158,117,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,12,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,158,108,12,12,12,12,12,15,12,12,35,35,16,35,35,35,35,35,35,35,35,35,
35,35,72,35,104,105,104,105,102,103,106,91,12,12,12,12,12,12,12,12,35,35,160,103,104,102,103,104,102,103,104,105,
160,161,35,11,6,7,8,7,38,38,38,108,12,14,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
169,170,102,171,172,179,42,120,38,39,40,108,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
0,2,3,0,4,5,6,7,8,9,39,108,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
23,24,25,26,54,28,29,30,118,50,89,142,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
54,26,49,21,23,54,4,5,43,44,88,35,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
32,120,22,32,120,118,27,55,123,35,35,35,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
63,62,61,62,63,61,63,63,88,35,35,35,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,35,35,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,35,35,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35,
35,35,35,35,35,35,35,35,35,35,35,35,12,12,12,12,12,12,12,12,35,35,35,35,35,35,35,35,35,35,35,35
};

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
	Billboard(uint16_t colorNdx, Vec2i pos)
	{
		// Alloc tiles
		auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::LowSpriteBank);
		constexpr auto spriteShape = Sprite::Shape::tall16x32;
		constexpr auto numTiles = Sprite::GetNumTiles(spriteShape);
		m_tileNdx = tileBank.allocDTiles(numTiles);

		// Init shadow sprite
		m_sprite.Configure(Sprite::ObjectMode::Normal, Sprite::GfxMode::Normal, Sprite::ColorMode::Palette256, spriteShape);
		m_sprite.SetNonAffineTransform(false, false, spriteShape);
		m_sprite.setDTiles(m_tileNdx);
		m_sprite.setPos(pos.x, pos.y);

		// Draw into the tiles
		for(uint32_t t = 0; t < numTiles; ++t)
		{
			auto& tile = tileBank.GetDTile(m_tileNdx + t);
			tile.fill(colorNdx);
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

struct Player
{
	Player(uint16_t colorNdx, Vec2i pos)
		: m_Billboard(colorNdx, pos)
		, m_Pos(pos)
	{}

	void Update()
	{
		if(Keypad::Held(Keypad::LEFT))
		{
			m_Pos.x -= 1;
		}
		if(Keypad::Held(Keypad::RIGHT))
		{
			m_Pos.x += 1;
		}
		m_Pos.x = max(0, min(240-16, m_Pos.x));
		
		if(Keypad::Held(Keypad::UP))
		{
			m_Pos.y -= 1;
		}
		if(Keypad::Held(Keypad::DOWN))
		{
			m_Pos.y += 1;
		}
		m_Pos.y = max(0, min(160-32, m_Pos.y));
		m_Billboard.m_sprite.setPos(m_Pos.x, m_Pos.y);
	}

	void Render(SpriteLinearAllocator& spriteAlloc)
	{
		m_Billboard.Render(spriteAlloc);
	}

	Billboard m_Billboard;
	Vec2i m_Pos;
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
	gfx::BackgroundPalette::Allocator::alloc(2*37-1);
	memcpy(gfx::BackgroundPalette::rawMemory(), talePalette, 37*4);
	GenerateBasicPalette<gfx::BackgroundPalette>();
	GenerateBasicPalette<gfx::SpritePalette>();

	// Set up mode 0, 256x256 tiles, 256 color palette
	Display().SetMode<0, DisplayControl::BG0>();
	Display().setupBackground(0,0,16,DisplayControl::TiledBGSize::e256x256);
}

void loadMapTileSet()
{
	auto& bgTiles = gfx::TileBank::GetBank(0);
	auto mapTileStart = bgTiles.allocDTiles(taleTileCount);
	auto mapMemory = bgTiles.GetDTileMemory(mapTileStart);
	memcpy(mapMemory, taleTileData, taleTileSize*4);
}

void loadMapData()
{
	auto bgMap = (uint16_t*)gfx::TileBank::GetBank(2).memory();
	memcpy(bgMap, kRiverMap, 32*32*2);
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

	Vec2i playerPos = {120-8, 80-8};
	auto player = Player(30, playerPos);
	//auto tree = Billboard(25, {180-8, 70-8});
	SpriteLinearAllocator spriteAlloc(32);

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while (1)
	{
		// Update
		Keypad::Update();
		
		player.Update();

		// Render
		spriteAlloc.reset();

		VBlankIntrWait();

		player.Render(spriteAlloc);
		//tree.Render(spriteAlloc);
	}
	return 0;
}
