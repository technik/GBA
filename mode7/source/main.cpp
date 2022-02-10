#include <cstdint>
#include <linearMath.h>
#include <Display.h>
#include <Device.h>
#include <Draw.h>
#include <Timer.h>
#include <cmath>
#include <tiles.h>
#include <Text.h>
#include <Keypad.h>
#include <tonc_math.h>

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

	// Prepare the background tile map
	BackgroundPalette()[1].raw = BasicColor::White.raw;
	BackgroundPalette()[2].raw = BasicColor::Red.raw;

	// Config BG2
	// Use charblock 0 for the tiles
	// Use the first screen block after charblock 0 (i.e. screenblock 8)
	// 128*128 map size
	IO::BG2CNT::Get().value =
		(1<<7) | // 16 bit color
		(8<<8); // screenblock 8

	// Fill in a couple tiles in video memory
    auto& tile0 = Sprite::DTileBlock(0)[0];
	tile0.fill(1); // White
	auto& tile1 = Sprite::DTileBlock(0)[1];
	tile1.fill(2); // Red

	// Fill in map data
	// Affine maps use 8 bit indices
	auto* mapMem = reinterpret_cast<volatile uint16_t*>(VideoMemAddress+0x4000);
	for(int32_t y = 0; y < 16; ++y)
	{
		for(int32_t x = 0; x < 8; ++x)
		{
			mapMem[y*8+x] = (y&1) ? 1 : (1<<8);
		}
	}

	Display().enableSprites();
	Display().EndBlank();

	int16_t rot = 0;
	intp12 depth = 1_p12;
	Vec2p12 bgPos = {};//Vec2p12(intp12(ScreenWidth/2), intp12(ScreenHeight/2));

	// Main loop
	int32_t t = 0;
	Timer0().reset<Timer::e1024>(); // Reset timer to ~1/16th of a millisecond
	while(1)
	{
		// Logic
		if(Keypad::Held(Keypad::KEY_LEFT))
		{
			bgPos.x() -= 1_p12;
		}
		if(Keypad::Held(Keypad::KEY_RIGHT))
		{
			bgPos.x() += 1_p12;
		}
		if(Keypad::Held(Keypad::KEY_UP))
		{
			bgPos.y() -= 1_p12;
		}
		if(Keypad::Held(Keypad::KEY_DOWN))
		{
			bgPos.y() += 1_p12;
		}
		if(Keypad::Held(Keypad::KEY_L))
		{
			depth += 0.01_p12;
		}
		if(Keypad::Held(Keypad::KEY_R))
		{
			depth -= 0.01_p12;
		}

		IO::BG2P::Get().refPoint.x() = (bgPos.x()*depth/240).cast_down<8>();
		IO::BG2P::Get().refPoint.y() = (bgPos.y()*depth/240).cast_down<8>();
		//IO::BG2P::Get().refPoint.x() = (bgPos.x()*depth).cast_down<8>();
		//IO::BG2P::Get().refPoint.y() = (bgPos.y()*depth).cast_down<8>();

		//int32_t sx = (lu_sin(rot)>>4)*128/240;
		//int32_t cx = (lu_cos(rot)>>4)*128/240;
		IO::BG2P::Get().tx.A = (2*depth.raw/160)/16;//(2*depth/160_p12).cast_down<8>().raw;//cx;
		IO::BG2P::Get().tx.B = 0;//sx;
		IO::BG2P::Get().tx.C = 0;//-sx;
		IO::BG2P::Get().tx.D = (2*depth/160).cast_down<8>().raw;//cx;

		// VSync
		Display().vSync();
		plotFrameIndicator();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond

		++t;
	}
	return 0;
}