#include <tools/frameCounter.h>
#include <gfx/tile.h>
#include <Text.h>
#include <cstring>

using namespace gfx;

FrameCounter::FrameCounter(TextSystem& text)
	: m_sprites(Sprite::ObjectAllocator::alloc(2))
{
	// Init the local sprites
	for(uint32_t i = 0; i < 2; ++i)
	{
		auto& obj = m_ShadowSprites[i];
		obj.attribute[0] = 1<<13; // Top of the screen, normal rendering, 16bit palette tiles
		obj.attribute[1] = 8*i; // Left of the screen, small size
	}
	const uint8_t data[2] = {};
	text.writeNumbers(data, m_ShadowSprites, 2);
}

void FrameCounter::render(TextSystem& text)
{
	// Separate digits
	auto fps = count();
	auto fps10 = fps/10;

	uint8_t counter[2] = {fps10, fps};

	// Draw frame rate indicator
	text.writeNumbers(counter, m_ShadowSprites, 2);

	// Copy over to VRAM
	memcpy(&m_sprites[0], &m_ShadowSprites[0], 3*sizeof(uint16_t));
	memcpy(&m_sprites[1], &m_ShadowSprites[1], 3*sizeof(uint16_t));
}

uint32_t FrameCounter::count()
{
	//uint32_t ms = Timer0().counter/16; // ~Milliseconds
	uint32_t tc = Timer0().counter;
	Timer0().reset<Timer::e1024>(); // Reset timer to 1/16th of a millisecond
	uint32_t fps = 60;
	if(tc > 16*256+3*64) // ~16.75, crude approx for 16.6667
	{
		fps = 30;
		if(tc > (33*256+128)) // 33.5 ms
		{
			fps = 20;
			if(tc > 50*256) // 66.75 ~= 66.6667
			{
				fps=15;
				if(tc > 66*256+3*64)
				{
					fps = 10;
					if(fps > 100*256)
					{
						fps = 0; // Slower than 10fps, don't bother
					}
				}
			}
		}
	}
	return fps;
}