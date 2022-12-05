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
#include <Keypad.h>
#include <linearMath.h>
#include <raycast.h>
#include <Text.h>
#include <Timer.h>
#include <gfx/palette.h>
#include <gfx/sprite.h>
#include <gfx/tile.h>
#include <tools/frameCounter.h>

// Demo code
#include <raycaster.h>
#include <Rasterizer.h>
#include <Camera.h>

// Levels
#include <test.wad.h>

using namespace math;
using namespace gfx;

TextSystem text;

// No need to place this method in fast memory
void Mode4Renderer::Init()
{
	Mode4Display displayMode;
	displayMode.Init();

	// Initialize the palette
	sPaletteStart = BackgroundPalette::Allocator::alloc(8);
	BackgroundPalette::color(sPaletteStart + 0).raw = BasicColor::SkyBlue.raw;
	BackgroundPalette::color(sPaletteStart + 1).raw = BasicColor::MidGrey.raw;
	// Wall colors
	BackgroundPalette::color(sPaletteStart + 2).raw = BasicColor::Green.raw;
	BackgroundPalette::color(sPaletteStart + 3).raw = BasicColor::DarkGreen.raw;
	BackgroundPalette::color(sPaletteStart + 4).raw = BasicColor::Black.raw;
	BackgroundPalette::color(sPaletteStart + 5).raw = BasicColor::DarkGrey.raw;
	BackgroundPalette::color(sPaletteStart + 6).raw = BasicColor::LightGrey.raw;
	// Player color
	BackgroundPalette::color(sPaletteStart + 7).raw = BasicColor::Yellow.raw;
}

void InitSystems()
{
	Display().enableSprites();
	// TextInit
	text.Init();

#ifdef GBA
	// Set up interrupts
	irq_init(NULL);
	// vblank int for vsync
	irq_add(II_VBLANK, NULL);
#endif
}

volatile uint32_t timerT2 = 0;

int main()
{
	// Full resolution, paletized color mode.
	Display().StartBlank();

	// Configure graphics
	Rasterizer::Init();

	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// -- Init game state ---
	auto camera = Camera(Rasterizer::DisplayMode::Width, Rasterizer::DisplayMode::Height, Vec3p16(0_p16, 0_p16, 1.7_p16));

	camera.m_halfClipHeight = Rasterizer::DisplayMode::Height / 2;
	camera.m_halfClipWidth = Rasterizer::DisplayMode::Width / 2;

	auto playerController = CharacterController(camera.m_pose);
	playerController.horSpeed = 0.06125_p16;
	playerController.angSpeed = 0.01_p16;

	// Unlock the display and start rendering
	Display().EndBlank();
	bool vBlank = true;

	// main loop
	while (1)
	{
		Timer1().reset<Timer::e64>(); // Set high precision profiler
		// Next frame logic
		Keypad::Update();
		playerController.update();
		// We're actually controlling the camera

		if (!Rasterizer::BeginFrame())
		{
			return 0; // Exit the app
		}

		// -- Render --
		Rasterizer::RenderWorld(camera);
#ifdef GBA
		frameCounter.render(text);

		timerT2 = Timer1().counter;

		// Present
		if (Keypad::Pressed(Keypad::R))
			vBlank = !vBlank;
		if (vBlank)
			VBlankIntrWait();
#endif
		Rasterizer::EndFrame();
	}
	return 0;
}
