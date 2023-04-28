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

void InitGraphics()
{
	// Init bg palette:
	// The first 16 colors store full bright primary colors
	auto primaries = gfx::BackgroundPalette::Allocator::alloc(15);
	// Some basic colors
	gfx::BackgroundPalette::color(primaries + 0) = BasicColor::Red;
	gfx::BackgroundPalette::color(primaries + 1) = BasicColor::Green;
	gfx::BackgroundPalette::color(primaries + 2) = BasicColor::Blue;
	gfx::BackgroundPalette::color(primaries + 3) = BasicColor::Orange;
	gfx::BackgroundPalette::color(primaries + 4) = BasicColor::Yellow;
	gfx::BackgroundPalette::color(primaries + 5) = BasicColor::Purple;
	gfx::BackgroundPalette::color(primaries + 6) = BasicColor::Pink;
	gfx::BackgroundPalette::color(primaries + 7) = BasicColor::LightGrey;
	gfx::BackgroundPalette::color(primaries + 8) = BasicColor::SkyBlue;
	// Setup a greyscale
	auto greyScale = gfx::BackgroundPalette::Allocator::alloc(32);
	for(auto i = 0; i < 32; ++i)
	{
		gfx::BackgroundPalette::color(greyScale + i) = Color(i,i,i); 
	}

	// Set up mode 0, 256x256 tiles, 256 color palette
	Display().SetMode<0, DisplayControl::BG0>();
	Display().setupBackground(0,0,4,DisplayControl::TiledBGSize::e256x256);
}

int main()
{
	// Full resolution, paletized color mode.
	Display().StartBlank();

	InitGraphics();

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while (1)
	{
	}
	return 0;
}
