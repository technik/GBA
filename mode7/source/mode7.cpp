// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
#include <tonc.h>

// Engine code
#include <Color.h>
#include <Device.h>
#include <Display.h>
#include <Keypad.h>
#include <linearMath.h>
#include <Text.h>
#include <Timer.h>
#include <gfx/palette.h>
#include <gfx/sprite.h>
#include <gfx/tile.h>
#include <tools/frameCounter.h>

// Demo code
#include <demo.h>
#include <Camera.h>

// Global Camera State
VECTOR gCamPos;
FIXED gCosf = 1<<8;
FIXED gSinf = 0;

TextSystem text;

void initBackground()
{
	// Background clear color (used for blending too)
	gfx::BackgroundPalette::color(0).raw = BasicColor::SkyBlue.raw;
	// Prepare the background tile map
	auto paletteStart = gfx::BackgroundPalette::Allocator::alloc(2);
	gfx::BackgroundPalette::color(paletteStart + 0).raw = BasicColor::White.raw;
	gfx::BackgroundPalette::color(paletteStart + 1).raw = BasicColor::Red.raw;

	// Config BG2
	// Use charblock 0 for the tiles
	// Use the first screen block after charblock 0 (i.e. screenblock 8)
	// 128*128 map size
	IO::BG2CNT::Get().value =
		(1<<7) | // 16 bit color
		(8<<8) | // screenblock 8
		(3<<0xe); // size 1024*1024

	// Fill in a couple tiles in video memory
	auto& tileBank = gfx::TileBank::GetBank(0);
	auto tileStart = tileBank.allocSTiles(2);
    auto& tile0 = tileBank.GetSTile(tileStart + 0);
	tile0.fill(1); // White
	auto& tile1 = tileBank.GetSTile(tileStart + 1);
	tile1.fill(2); // Red

	// Fill in map data
	// Affine maps use 8 bit indices
	auto* mapMem = reinterpret_cast<volatile uint16_t*>(VideoMemAddress+0x4000);
	for(int32_t y = 0; y < 128; ++y)
	{
		for(int32_t x = 0; x < 64; ++x)
		{
			mapMem[y*64+x] = (y&1) ? 1 : (1<<8);
		}
	}
}

struct RasteredObj
{
	RasteredObj(VECTOR startPos)
		:m_pos(startPos)
	{
		m_paletteStart = gfx::SpritePalette::Allocator::alloc(1); // Alloc 1 color

		// Init palette
		gfx::SpritePalette::color(m_paletteStart).raw = BasicColor::Green.raw;

		// Init sprite
		auto& tileBank = gfx::TileBank::GetBank(gfx::TileBank::LowSpriteBank);
		m_sprite = Sprite::ObjectAllocator::alloc(1);
		m_tileNdx = tileBank.allocSTiles(1);
		m_sprite->attribute[2] = m_tileNdx;
		m_sprite->setPos(116, 76);

		// Draw into the tile
		constexpr uint32_t lowBank = 4;
		auto* spriteBase = (volatile uint16_t*)tileBank.GetSTile(m_tileNdx).pixelPair;
		uint32_t fourTiles = (m_paletteStart<<8) | m_paletteStart;
		for(uint16_t i = 0; i < 8*4; ++i) // Fill the tile
		{
			spriteBase[i] = (i&2) ? fourTiles : (fourTiles<<4);
		}
	}

	void update(const Camera& cam)
	{
		VECTOR relPos;
		relPos.x = m_pos.x-cam.pos.x;
		relPos.y = m_pos.y-cam.pos.y;
		relPos.z = m_pos.z-cam.pos.z;
		//VECTOR vsPos = relPos.x*
	}

	void render()
	{}

	VECTOR m_pos;
    uint32_t m_paletteStart;
	volatile Sprite::Object* m_sprite {};
	uint32_t m_tileNdx = 0;
};

void resetBg2Projection()
{
	REG_BG2PA = 0;
	REG_BG2PC = 0;
	REG_BG2X = -1*(1<<8);
	REG_BG2Y = -1*(1<<8);
}

void InitSystems()
{
	// Init mode7 background
	Display().enableSprites();

	// TextInit
	text.Init();
	
	// enable hblank register and set the mode7 type
	irq_init(NULL);
	irq_add(II_HBLANK, (fnptr)m7_hbl_c);
	// and vblank int for vsync
	irq_add(II_VBLANK, NULL);
}

int main()
{
	Display().StartBlank();
	Display().InitMode2();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	initBackground();

	// -- Init game state ---
	auto camera = Camera({ 256<<8, 256<<8, 2<<8 });

	// Create a 3d object in front of the camera
	VECTOR objPos = camera.pos;
	objPos.y += 5;
	auto obj0 = RasteredObj(objPos);
	
	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		camera.update();
		obj0.update(camera);

		// VSync
		VBlankIntrWait();

		// -- Render --
		// Operations should be ordered from most to least time critical, in case they exceed VBlank time
		resetBg2Projection();
		camera.postGlobalState();
		// Prepare first scanline for next frame

		frameCounter.render(text);
	}
	return 0;
}
