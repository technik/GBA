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

constexpr int kCellSize = 64;
constexpr int kMapRows = 8;
constexpr int kMapCols = 8;
uint8_t map[kMapRows * kMapCols] = {
	1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1,
};

void Render(const Camera& cam)
{
	// Reconstruct local axes for fast ray interpolation
	Vec2p8 xDir = { cam.m_pose.cosf/2, cam.m_pose.sinf/2 };
	Vec2p8 yDir = { -cam.m_pose.sinf, cam.m_pose.cosf };
	Vec2p8 minRay = yDir - xDir;
	intp12 recip = math::Fixed<int32_t, 12>::castFromShiftedInteger<12>(lu_div(Mode4Display::Width/2));

	for(int col = 0; col < Mode4Display::Width; ++col)
	{
		// Compute a ray direction for this column
		auto deltaX = xDir * (col * recip);
		Vec2p8 rayDir = minRay + Vec2p8{deltaX.x().cast<8>(), deltaX.y().cast<8>()};

		// 1: Find intersection distance
		intp8 distance = kCellSize * 15_p8; // Bigger than the map's diagonal
		intp16 distance2 = distance*distance; // Distance square for faster compare ops
		// ray is of the form: y = y0 + (x-x0) * m
		// Follow a line rasterization algorithm to find the closest intersection.
		if(abs(rayDir.x().raw) > abs(rayDir.y().raw))
		{ // Iterate on x increments
		}
		else{ // Iterate on y increments

		}
		// Intersect horizontal walls
		if(rayDir.x().raw != 0)
		{
			int xmin, xmax;
			int camFloor = cam.m_pose.pos.x().floor() + 1;
			if(rayDir.x() < 0_p8)
			{
				xmin = 0;
				xmax = min(camFloor+1, kMapCols-1);
			}
			else
			{
				xmin = max(0, cam.m_pose.pos.x().floor());
				xmax = kMapCols-1;
			}
			for(int xWall = xmin; xWall <= xmax; ++xWall)
			{
			}
		}
		// Intersect vertical walls
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
	auto camera = Camera(ScreenWidth, ScreenHeight, Vec3p8(128_p8, 128_p8, 0_p8));
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
