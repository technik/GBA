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

struct Mesh
{
	Vec3p8* vertices;
	uint16_t* indices;
	Color* faceColors;
	uint16_t numTris;
	uint16_t numVertices;
};

void DrawIndexedMesh(const YawPitchCamera& cam, const Mat34p16& worldMtx, const Mesh& mesh)
{
	constexpr int MAX_VERTICES = 16;
	dbgAssert(mesh.numVertices <= MAX_VERTICES);

	// Concat matrices
	//Mat34p16 worldView = cam.worldView(worldMtx);
	//
	//Vec3p8 vertices
}


void DrawPyramid(const YawPitchCamera& cam)
{
	const Vec3p8 vertices[5] = {
		{-0.5_p8,-0.5_p8, 0.5_p8},
		{-0.5_p8, 0.5_p8, 0.5_p8},
		{ 0.5_p8, 0.5_p8, 0.5_p8},
		{ 0.5_p8,-0.5_p8, 0.5_p8},
		{ 0.0_p8, 0.0_p8, 0.5_p8}
	};

	const uint16_t indices[12] = {
		1, 0, 4,
		0, 3, 4,
		3, 2, 4,
		2, 1, 4
	};

	const Color faceColors[4] = {
		BasicColor::Red,
		BasicColor::Green,
		BasicColor::Blue,
		BasicColor::Yellow
	};
}

void clearBg(uint16_t* buffer, uint16_t topClr, uint16_t bottomClr, int area)
{
	DMA::Channel0().Fill(&buffer[0 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[1 * area / 4], topClr, area / 4);
	DMA::Channel0().Fill(&buffer[2 * area / 4], bottomClr, area / 4);
	DMA::Channel0().Fill(&buffer[3 * area / 4], bottomClr, area / 4);
}

void RenderWorld(const YawPitchCamera& cam)
{
	clearBg(Display().backBuffer(), Rasterizer::skyClr.raw, Rasterizer::groundClr.raw, Mode5Display::Area);

	const Vec3p8 vertices[3] = {
		{-0.5_p8, 2.5_p8, -0.5_p8},
		{ 0.5_p8, 2.5_p8, -0.5_p8},
		{ 0_p8,   2.5_p8, 0.5_p8},
	};

	Vec2p16 ssVertices[3];
	Vec2p8 ssVertices8[3];
	for (int i = 0; i < 3; ++i)
	{
		Vec3p8 vsVtx = cam.transformPos(vertices[i]);
		Vec3p8 csVtx = vsVtx.y == 0 ? Vec3p8{} : projectPosition(vsVtx);
		ssVertices8[i] = {
			(csVtx.x * 80 + 80),
			(csVtx.y * 64 + 64)
		};
		ssVertices[i] = {
			ssVertices8[i].x.cast<16>(),
			ssVertices8[i].y.cast<16>()
		};
	}

	rasterTriangle(Display().backBuffer(), { 160, 128 }, BasicColor::Green.raw, ssVertices8);
}

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
	auto camera = YawPitchCamera();

	auto horSpeed = 0.06125_p16;
	auto angSpeed = 0.01_p16;

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
		camera.update(horSpeed, angSpeed);
		camera.refreshRot();
		// We're actually controlling the camera

		if (!Rasterizer::BeginFrame())
		{
			return 0; // Exit the app
		}

		// -- Render --
		RenderWorld(camera);
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
