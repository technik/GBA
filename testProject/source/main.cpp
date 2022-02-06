#include <cstdint>
#include "linearMath.h"
#include "Display.h"
#include "Draw.h"
#include "Timer.h"
#include <cmath>
#include "tiles.h"

using namespace math;

extern const uint32_t fontTileDataSize;
extern const uint32_t fontTileData[];

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

void trace(uint16_t* backBuffer, int32_t t)
{
	constexpr uint32_t bgBlue = (0x1f<<10)|(0x1f<<5)|(160>>3);

	Vec2p8 uv;
	constexpr auto invY = 2_p8/ScreenHeight;
	constexpr auto invX = 2_p8/ScreenWidth;

	for(int32_t y = 0; y < ScreenHeight; ++y)
	{
		uv.y() = (intp8(y)*invY-1_p8)*3;
		auto y2_dst = uv.y()*uv.y()-0.5_p8;

		for(int32_t x = 0; x < ScreenWidth; ++x)
		{
			int32_t pixel = x+y*ScreenWidth;
			uv.x() = (intp8(x)*invX-1_p8)*4;

			auto dst = y2_dst+(uv.x()*uv.x());
			if(dst.raw < 0)
			{
				backBuffer[pixel] = 0xefff;
			}
			else
			{
				backBuffer[pixel] = bgBlue;
			}
		}
	}
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

	// Set up color palette
	auto palette = SpritePalette();
	palette[0] = BasicColor::White; 
	palette[1] = BasicColor::Black;
	palette[2] = BasicColor::White;
	palette[3] = BasicColor::Blue;

	// Fill the first tiles in VRAM
	Sprite::DTileBlock(5)[0].fill(1); // Zero would be transparent
	Sprite::DTileBlock(5)[1].fill(2);
	Sprite::DTileBlock(5)[2].fill(3);

	// Copy the font
	auto* spriteBase = ((volatile uint16_t*)Sprite::DTileBlock(5));
	auto* src = reinterpret_cast<const uint16_t*>(fontTileData);
	for(uint32_t i = 0; i < 2*fontTileDataSize; ++i)
	{
		spriteBase[i] = src[i];//0x01010201;//fontTileData[i];
		//spriteBase[i] = 0x01010201;//fontTileData[i];
	}

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
		//uint16_t backBuffer[ScreenWidth*ScreenHeight];
		drawScene(displayBuffer, t);
		//trace(displayBuffer, t);

		// VSync
		plotFrameIndicator(displayBuffer);
		Display().vSync();
		Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
		Display().flipFrame();

		++t;
	}
	return 0;
}