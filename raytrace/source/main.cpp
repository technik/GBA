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
using namespace gfx;

TextSystem text;

struct Sphere
{
	Vec3p8 pos;
	intp8 radius;
};

void initBackground()
{
	// Initialize the palette
	auto paletteNdx = BackgroundPalette::Allocator::alloc(2);
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Red.raw;
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Blue.raw;
}

void InitSystems()
{
	Display().enableSprites();
	// TextInit
	text.Init();
}

// returns the background palette index with this pixel's color
uint16_t trace(const Camera& cam, uint32_t x, uint32_t y)
{
	return 2;
}

void Render(const Camera& cam)
{
	auto backBuffer = DisplayControl::Get().backBuffer();
	for(int y = 0; y < Mode4Display::Height; ++y)
	{
		auto row = &backBuffer[Mode4Display::Width/2 * y];
		for(int x = 0; x < Mode4Display::Width/2; ++x)
		{
			auto pixelA = trace(cam, 2*x,y);
			auto pixelB = trace(cam, 2*x+1,y);
			row[x] = (pixelA + (pixelB<<8));
		}
	}
}

int main()
{
	Display().StartBlank();

	// Full resolution, paletized color mode.
	Mode4Display mode4;
	mode4.Init();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	initBackground();

	// -- Init game state ---
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(0_p8, 0_p8, 0_p8));
	auto playerController = CharacterController(camera.m_pose);

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		// Next frame logic
		Keypad::Update();
		playerController.update();

		// -- Render --
		Render(camera);
		frameCounter.render(text);

		// Present
		Display().flipFrame();
	}
	return 0;
}
