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
}

void InitSystems()
{
	Display().enableSprites();
	// TextInit
	text.Init();
}

int main()
{
	Display().StartBlank();
	Display().SetMode<2,DisplayControl::BG2>();
	
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
