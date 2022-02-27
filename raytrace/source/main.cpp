// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>

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

using namespace math;

TextSystem text;

void initBackground()
{
	// Background clear color (used for blending too)
	gfx::BackgroundPalette::color(0).raw = BasicColor::SkyBlue.raw;
	// Prepare the background tile map
	uint32_t numColors = 4; // 2 in tile colors + 2 border colors
	auto paletteStart = gfx::BackgroundPalette::Allocator::alloc(2);
	gfx::BackgroundPalette::color(paletteStart + 0).raw = BasicColor::White.raw;
	gfx::BackgroundPalette::color(paletteStart + 1).raw = BasicColor::Red.raw;

	// Config BG2
	// Use charblock 0 for the tiles
	// Use the first screen block after charblock 0 (i.e. screenblock 8)
	// 128*128 map size
	IO::BG2CNT::Get().value =
		(1<<7) | // 16 bit color, technically not necessary as Affine backgrounds are always 16bit colors
		(8<<8) | // screenblock 8
		(3<<0xe); // size 1024*1024

	// Fill in a couple tiles in video memory
	auto& tileBank = gfx::TileBank::GetBank(0);
	auto tileStart = tileBank.allocDTiles(2);
    auto& tile0 = tileBank.GetDTile(tileStart + 0);
	tile0.fill(1); // White
	auto& tile1 = tileBank.GetDTile(tileStart + 1);
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

void InitSystems()
{
	// TextInit
	text.Init();
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
	auto camera = Camera(Vec3p8(256_p8, 256_p8, 1.7_p8));
	
	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		camera.update();

		// VSync
		VBlankIntrWait();

		// -- Render --
		frameCounter.render(text);
	}
	return 0;
}
