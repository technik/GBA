#include <cstdint>
#include <linearMath.h>
#include <Display.h>
#include <Draw.h>
#include <Timer.h>
#include <cmath>
#include <tiles.h>
#include <Text.h>

using namespace math;

void plotFrameIndicator()
{
	// Draw frame rate indicator
	uint32_t ms = Timer0().counter/16; // ~Milliseconds
	auto* tile0 = &Sprite::OAM()[0].objects[0];
	auto ms10 = ms/10;
	tile0->attribute[2] = Sprite::DTile::HighSpriteBankIndex(ms10+16);
	auto* tile1 = &Sprite::OAM()[0].objects[1];
	tile1->attribute[2] = Sprite::DTile::HighSpriteBankIndex(ms-10*ms10+16);
}

int main()
{
	Display().StartBlank();
	Display().InitMode2();

	// TextInit
	TextSystem text;
	text.Init();

	// Init the frame counter
	auto* obj0 = &Sprite::OAM()[0].objects[0];
	obj0->attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
	obj0->attribute[1] = 0; // Left of the screen, small size
	obj0->attribute[2] = Sprite::DTile::HighSpriteBankIndex('0'-32);
	auto* obj1 = &Sprite::OAM()[0].objects[1];
	obj1->attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
	obj1->attribute[1] = 8; // Left of the screen, small size
	obj1->attribute[2] = Sprite::DTile::HighSpriteBankIndex('1'-32);

	// Prepare the background tile

	Display().enableSprites();
	Display().EndBlank();

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to ~1/16th of a millisecond
	while(1)
	{
		// Logic

		// VSync
		plotFrameIndicator();
		Display().vSync();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond

		++t;
	}
	return 0;
}