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
#include <matrix.h>

// Levels
#include <test.wad.h>

using namespace math;
using namespace gfx;

TextSystem text;

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

	// FOV = 66 deg
	auto xFocalLen = 1.5398649638145827_p16; // 1/tan(radians(66)/2)
	auto yFocalLen = 2.309797445721874_p16; // 1/tan(radians(66)/2) * 240/160

	auto proj = math::projectionMatrix(xFocalLen, yFocalLen, intp16(1/128.0));

	// Unlock the display and start rendering
	Display().EndBlank();
	bool vBlank = true;

	// main loop
	int tx = 0;
	while (1)
	{
		Timer1().reset<Timer::e64>(); // Set high precision profiler
		// Next frame logic
		Keypad::Update();
		playerController.update();
		// We're actually controlling the camera

		intp16 sx = intp16::castFromShiftedInteger<12>(lu_sin(tx));
		intp16 cx = intp16::castFromShiftedInteger<12>(lu_sin(tx));
		tx += 100;

		math::Mat34p16 modelMtx = {
			cx, 0_p16, -sx, 0_p16,
			0_p16, 1_p16, 0_p16, 0_p16,
			sx, 0_p16, cx, -4_p16
		};

		auto mvp = proj * modelMtx;

		if (!Rasterizer::BeginFrame())
		{
			return 0; // Exit the app
		}

		// -- Render --
		Rasterizer::RenderWorld(camera, mvp);
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
