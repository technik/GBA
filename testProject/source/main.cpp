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
	while(REG_VCOUNT < 160)
	{}
}

bool tSphere(uint32_t x, uint32_t y, float fov, float rad, float depth)
{
	return false;
}

int main()
{
	DisplayControl::Get().set<3, DisplayControl::BG2>();
	//set GBA rendering context to MODE 3 Bitmap Rendering
	//REG_DISPLAY_CTRL = VIDEOMODE_3 | BG_ENABLE2;

	int32_t t = 0;

	constexpr uint8_t screenWidth = 240;
	constexpr uint8_t screenHeight = 160;

	// Precalc the palette
	for(uint32_t i = 0; i < 256; ++i)
	{
		uint32_t bottom = i&0xf;
		uint32_t top = i>>4;
		//REG_PALETTE[i] = (top<<16)|(top<<12)|(bottom<<4)|bottom;
	}

	// Main loop
	while(1)
	{
		// Logic
		uint32_t color = (t&0x1F)<<10;

		// VSync
		vsync();

		// Draw
		for(int32_t y = 0; y < screenHeight; ++y)
		{
			uint32_t green = y>>3;
			color = (color&(0x1f<<10)) | (green<<5);
			uint32_t* line = &REG_SCREEN_BUFFER_2[y*120];

			for(int32_t x = 0; x < 120; x++)
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