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

void TriangulateScreen()
{}

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
#if 0
		rasterTriangle(
			Display().backBuffer(), { 160, 128 },
			mesh.faceColors[i].raw,
			vertices);
#else
		rasterTriangleExp(
			Display().backBuffer(), { 160, 128 },
			mesh.faceColors[i].raw,
			vertices);
	}
#endif
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

void DrawTestScreen()
{
	constexpr intp8 xOffset = 0_p8;
	constexpr intp8 yOffset = 0_p8;

	constexpr Vec2p8 vertices[12] = {
		{   1.5_p8 + xOffset,   1.5_p8 + yOffset },
		{  32.5_p8 + xOffset,   1.5_p8 + yOffset },
		{  64.0_p8 + xOffset,   1.5_p8 + yOffset },
		{ 158.5_p8 + xOffset,   1.5_p8 + yOffset },
		{   1.5_p8 + xOffset,  64.0_p8 + yOffset },
		{  96.0_p8 + xOffset,  64.0_p8 + yOffset },
		{ 127.5_p8 + xOffset,  64.0_p8 + yOffset },
		{ 158.5_p8 + xOffset,  64.0_p8 + yOffset },
		{   1.5_p8 + xOffset, 126.5_p8 + yOffset },
		{  32.5_p8 + xOffset, 126.5_p8 + yOffset },
		{  64.0_p8 + xOffset, 126.5_p8 + yOffset },
		{ 158.5_p8 + xOffset, 126.5_p8 + yOffset },
	};

	const uint16_t indices[12 * 3] = {
		 4, 1, 0,
		 4, 2, 1,
		 4, 3, 2,
		 4, 5, 3,
		 5, 6, 3,
		 6, 7, 3,
		11, 7, 6,
		11, 6, 5,
		11, 5, 4,
		10,11, 4,
		 9,10, 4,
		 8, 9, 4
	};

	for (int i = 0; i < 12; ++i)
	{
		Vec2p8 triVerts[3] = {
			vertices[indices[3*i+0]],
			vertices[indices[3*i+1]],
			vertices[indices[3*i+2]],
		};

		//rasterTriangle(
		//	Display().backBuffer(), { 160, 128 },
		//	Squirrel3(i),
		//	triVerts);
		rasterTriangleExp(
			Display().backBuffer(), { 160, 128 },
			Squirrel3(i),
			triVerts);
	}
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
	//DrawTestScreen();

	DrawPyramid(cam);
}

class SoundControlIO final
{
public:
	// Singleton access
    inline static SoundControlIO& Get() { return *IO::GlobalMemory<SoundControlIO,IO::SOUND1CNT_L::address>(); }
	SoundControlIO() = delete; // Prevent instantiation

	volatile uint16_t SOUND1CNT_L;
	volatile uint16_t SOUND1CNT_H;
	volatile uint16_t SOUND1CNT_X;
	volatile uint16_t pad0; // 0x4000066 unused
	volatile uint16_t SOUND2CNT_L;
	uint16_t pad1; // 0x400006A unused
	volatile uint16_t SOUND2CNT_H;
	uint16_t pad2; // 0x400006E unused
	volatile uint16_t SOUND3CNT_L;
	volatile uint16_t SOUND3CNT_H;
	volatile uint16_t SOUND3CNT_X;
	uint16_t pad3; // 0x4000076 unused
	volatile uint16_t SOUND4CNT_L;
	uint16_t pad4; // 0x400007A unused
	volatile uint16_t SOUND4CNT_H;
	uint16_t pad5; // 0x400007E unused
	volatile uint16_t SOUNDCNT_L;
	volatile uint16_t SOUNDCNT_H;
	volatile uint16_t SOUNDCNT_X;
	uint16_t pad6; // 0x4000086 unused
	volatile uint16_t SOUNDBIAS;
};

void soundPlay()
{
	auto& snd = SoundControlIO::Get();
	snd.SOUNDCNT_X = 1<<7;
	snd.SOUND1CNT_L = 1<<3;
	snd.SOUND1CNT_H = (1<<7) | (1<<0xb);
	snd.SOUND1CNT_X = 194;

	//if (Keypad::Held(Keypad::A))
		snd.SOUNDCNT_L = 7 | (7<<4) | (1<<8)| (1<<12);
		snd.SOUNDCNT_H = 2;
	//else
	//	snd.SOUNDCNT_L = 0;
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
	camera.pos = Vec3p8(0_p8, -2_p8, 0_p8);

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

		// Sound
		soundPlay();

		// -- Render --
		clearBg(Display().backBuffer(), Rasterizer::skyClr.raw, Rasterizer::groundClr.raw, Mode5Display::Area);

		Timer1().reset<Timer::e64>(); // Set high precision profiler
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
