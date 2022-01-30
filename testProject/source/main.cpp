#include <cstdint>
#include "Display.h"

// Hardware definition
#define REG_DISPLAY_CTRL *(unsigned int*)0x04000000
#define REG_SCREEN_BUFFER ((unsigned short*)0x06000000)
#define REG_SCREEN_BUFFER_2 ((uint32_t*)0x06000000)
#define VIDEOMODE_3 0x0003
#define BG_ENABLE2 0x0400
#define REG_VCOUNT (*(volatile uint16_t*)(0x04000006))
#define REG_PALETTE (*(uint16_t*)(0x05000000))



void vsync()
{
	while(REG_VCOUNT > 160)
	{}
	while(REG_VCOUNT <= 160)
	{}
}

int main()
{
#if 0
	constexpr uint32_t ScreenWidth = 240;
	constexpr uint32_t ScreenHeight = 160;
	constexpr uint16_t VideoMode = 3;
#else
	constexpr uint32_t ScreenWidth = 160;
	constexpr uint32_t ScreenHeight = 128;
	constexpr uint16_t VideoMode = 5;
#endif

	DisplayControl::Get().set<VideoMode, DisplayControl::BG2>();
	//set GBA rendering context to MODE 3 Bitmap Rendering
	//REG_DISPLAY_CTRL = VIDEOMODE_3 | BG_ENABLE2;

	int32_t t = 0;


	// Main loop
	while(1)
	{
		// Logic
		uint32_t color = (t&0x1F)<<10;

		// VSync
		vsync();
		DisplayControl::Get().flipFrame();
		uint16_t* backBuffer = DisplayControl::Get().backBuffer();

		// Draw
		for(int32_t y = 0; y < ScreenHeight; ++y)
		{
			uint32_t green = y>>3;
			color = (color&(0x1f<<10)) | (green<<5);
			uint32_t* line = &((uint32_t*)backBuffer)[y*(ScreenWidth/2)];

			for(int32_t x = 0; x < (ScreenWidth/2); x++)
			{
				uint32_t red = x>>2;
				uint32_t pixelColor = red | color;
				line[x] = pixelColor<<16 | pixelColor;
			}
		}
		++t;
	}
	return 0;
}