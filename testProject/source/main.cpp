#include <cstdint>
#include "linearMath.h"
#include "Display.h"
#include "Draw.h"
#include "Timer.h"
#include <cmath>
#include "tiles.h"
#include "Text.h"

using namespace math;

void plotFrameIndicator(uint16_t* frameBuffer)
{
	// Draw frame rate indicator
	uint32_t ms = Timer0().counter/16; // ~Milliseconds
	auto* tile0 = &Sprite::OAM()[0].objects[0];
	auto ms10 = ms/10;
	tile0->attribute[2] = Sprite::DTile::HighSpriteBankIndex(ms10+16);
	auto* tile1 = &Sprite::OAM()[0].objects[1];
	tile1->attribute[2] = Sprite::DTile::HighSpriteBankIndex(ms-10*ms10+16);
}

void drawScene(uint16_t* backBuffer, int32_t t)
{
	constexpr uint32_t bgBlue = (0x1f<<10)|(0x1f<<5)|(160>>3);
	// Draw
	clear(backBuffer, bgBlue);
	constexpr auto halfSide = 20_p8;
	constexpr auto center = 100_p8;
	auto sx = intp8((float)sin(t*0.125f));
	auto ndl = max(0_p8, intp8((float)sin(t*0.125f+0.2f)));

	math::Vec2p8 tri[3] = {
		math::Vec2p8(center-halfSide*sx,100_p8),
		math::Vec2p8(center+halfSide*sx,100_p8),
		math::Vec2p8(center,40_p8)
	};

	rasterTriangle(backBuffer, Color(0,(int)(0x1f*(1+ndl)/2).round(),0).raw, tri);
}

int main()
{
	Display().StartBlank();
	Display().Init();

	// TextInit
	TextSystem text;
	text.Init();

	// Create two sprites using with them
	auto* obj0 = &Sprite::OAM()[0].objects[0];
	obj0->attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
	obj0->attribute[1] = 0; // Left of the screen, small size
	obj0->attribute[2] = Sprite::DTile::HighSpriteBankIndex('0'-32);
	auto* obj1 = &Sprite::OAM()[0].objects[1];
	obj1->attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
	obj1->attribute[1] = 8; // Left of the screen, small size
	obj1->attribute[2] = Sprite::DTile::HighSpriteBankIndex('1'-32);

	Display().enableSprites();
	Display().EndBlank();

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to ~1/16th of a millisecond
	while(1)
	{
		auto* displayBuffer = Display().backBuffer();
		// Logic
		drawScene(displayBuffer, t);

		// VSync
		plotFrameIndicator(displayBuffer);
		Display().vSync();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
		Display().flipFrame();

		++t;
	}
	return 0;
}