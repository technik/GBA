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
	const Vec3p8* vertices;
	const uint16_t* indices;
	const Color* faceColors;
	uint16_t numTris;
	uint16_t numVertices;
};

void DrawStaticIndexedMesh(const YawPitchCamera& cam, const Mesh& mesh)
{
	constexpr int MAX_VERTICES = 16;
	dbgAssert(mesh.numVertices <= MAX_VERTICES);

	// Transform vertices to screen space
	Vec2p8 projVertices[MAX_VERTICES];
	for (int i = 0; i < mesh.numVertices; ++i)
	{
		Vec3p8 vsVtx = cam.transformPos(mesh.vertices[i]);
		Vec3p8 csVtx = vsVtx.y == 0 ? Vec3p8{} : projectPosition(vsVtx);
		projVertices[i] = {
			(csVtx.x * 80 + 80),
			(csVtx.y * 64 + 64)
		};
	}

	// Draw triangles
	int baseIndex = 0;
	for (int i = 0; i < mesh.numTris; ++i)
	{
		Vec2p8 vertices[3] =
		{
			projVertices[mesh.indices[baseIndex++]],
			projVertices[mesh.indices[baseIndex++]],
			projVertices[mesh.indices[baseIndex++]]
		};
		rasterTriangle(
			Display().backBuffer(), { 160, 128 },
			mesh.faceColors[i].raw,
			vertices);
	}
}

// Draws a floorless pyramid as static geometry (no local world transform)
void DrawPyramid(const YawPitchCamera& cam)
{
	const Vec3p8 vertices[5] = {
		{-0.5_p8,-0.5_p8, -0.5_p8},
		{-0.5_p8, 0.5_p8, -0.5_p8},
		{ 0.5_p8, 0.5_p8, -0.5_p8},
		{ 0.5_p8,-0.5_p8, -0.5_p8},
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

	const Mesh pyramid = {
		vertices,
		indices,
		faceColors,
		4,
		5
	};

	DrawStaticIndexedMesh(cam, pyramid);
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

	DrawPyramid(cam);
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
	camera.pos = Vec3p8(0_p8, -4_p8, 0_p8);

	auto horSpeed = 0.06125_p16;
	auto angSpeed = 0.001_p16;

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
