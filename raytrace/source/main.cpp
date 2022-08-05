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
uint16_t trace(const Camera& cam, int32_t x, int32_t y)
{
	// View space ray
	Vec3p8 rayDir;
	rayDir.z() = intp8(Mode4Display::Height/2-y)/int(Mode4Display::Width/2);
	intp8 vx = intp8(x-Mode4Display::Width/2)/int(Mode4Display::Width/2);
	// rotate around the camera
	rayDir.x() = (cam.m_pose.cosf * vx).cast<8>() - cam.m_pose.sinf;
	rayDir.y() = -(cam.m_pose.sinf * vx).cast<8>() - cam.m_pose.cosf;

	// Intersect ray against a unit sphere
	// p = p0 + t*d
	// p*p <= R2
	// p0*p0 + t^2*d*d + 2*t*p0*d <= R^2
	// t^2*|d|^2 + t * (2*dot(p0,d)) + (|p0|^2 - R^2) = 0
	// t = -b +- sqrt(b^2 - 4*a*c) / (2*a)
	// b^2 > 4*a*c -> 4*(p0*d)*(p0*d) > 4*(d*d * (p0*p0 - R*R))
	// (p0*d)*(p0*d) > (d*d * (p0*p0 - R*R))
	Vec3p8 p0 = cam.m_pose.pos - Vec3p8(0_p8,10_p8,0_p8);
	intp8 p0d = dot(p0, rayDir).cast<8>();
	auto dd = dot(rayDir,rayDir).cast<12>();
	intp8 p02 = dot(p0,p0).cast<8>();
	return ((p0d*p0d) > (dd * (p02 - 1_p8)).cast<16>()) ? 1 : 2;
}

void Render(const Camera& cam)
{
	auto backBuffer = DisplayControl::Get().backBuffer();
	for(int y = 0; y < Mode4Display::Height; ++y)
	{
		auto row = &backBuffer[Mode4Display::Width/2 * y];
		for(int x = 0; x < Mode4Display::Width/2; ++x)
		{
			auto pixel = trace(cam, 2*x + 1,y);
			row[x] = (pixel + (pixel<<8));
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
