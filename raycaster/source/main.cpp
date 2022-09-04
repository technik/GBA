// ----------------------------------------------------------------------------
// Perspective projection demo (a.k.a. Mode 7)
// ----------------------------------------------------------------------------

// External libraries
#include <stdio.h>
extern "C"
 {
#include <tonc_types.h>
 }

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
#include <Camera.h>

using namespace math;
using namespace gfx;

TextSystem text;

// No need to place this method in fast memory
void Mode4Renderer::Init()
{
	Mode4Display displayMode;
	displayMode.Init();

	// Initialize the palette
	auto paletteNdx = BackgroundPalette::Allocator::alloc(4);
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::SkyBlue.raw;
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::MidGrey.raw;
	// Wall color
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Green.raw;
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::DarkGreen.raw;
	// Player color
	BackgroundPalette::color(paletteNdx++).raw = BasicColor::Yellow.raw;
}

void InitSystems()
{
	Display().enableSprites();
	// TextInit
	text.Init();

	// Set up interrupts
	irq_init(NULL);
	// vblank int for vsync
	irq_add(II_VBLANK, NULL);
}

volatile uint32_t timerT2 = 0;

int main()
{
	// Full resolution, paletized color mode.
	Display().StartBlank();
	
	// --- Init systems ---
	InitSystems();
	FrameCounter frameCounter(text);

	// Configure graphics
	Mode4Renderer::Init();

	// -- Init game state ---
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(2.5_p8, 2.5_p8, 0_p8));
	auto playerController = CharacterController(camera.m_pose);
	playerController.horSpeed = 0.06125_p8;
	playerController.angSpeed = 0.01_p16;

	// Unlock the display and start rendering
	Display().EndBlank();

	// main loop
	while(1)
	{
		Timer1().reset<Timer::e64>(); // Set high precision profiler
		// Next frame logic
		Keypad::Update();
		playerController.update();
		playerController.m_pose.pos.x() = max(1.125_p8, playerController.m_pose.pos.x());
		playerController.m_pose.pos.y() = max(1.125_p8, playerController.m_pose.pos.y());
		playerController.m_pose.pos.x() = min(intp8(kMapCols) - 1.125_p8, playerController.m_pose.pos.x());
		playerController.m_pose.pos.y() = min(intp8(kMapRows) - 1.125_p8, playerController.m_pose.pos.y());

		// -- Render --
		Mode4Renderer::RenderWorld(camera);
		frameCounter.render(text);

		timerT2 = Timer1().counter;
		// Present
		VBlankIntrWait();
		Display().flipFrame();

		// Copy the render target
		// TODO: Use the dma here
		// Use DMA channel 3 so that we can copy data from external RAM. If we split the FB into chunks, we may be able to use IWRAM and channel 0.
		//auto backBuffer = (uint32_t*)DisplayControl::Get().backBuffer();
		//DMA::Channel3().Copy(backBuffer, src, Mode4Display::Width * Mode4Display::Height / 4);
	}
	return 0;
}
