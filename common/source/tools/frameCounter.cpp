#include <tools/frameCounter.h>
#include <gfx/tile.h>
#include <Text.h>
#include <cstring>

using namespace gfx;


FrameCounter::FrameCounter(TextSystem& text)
	: m_sprites(Sprite::ObjectAllocator::alloc(kNumDigits))
{
	// Init the local sprites
	for(uint32_t i = 0; i < kNumDigits; ++i)
	{
		auto& obj = m_DigitSprites[i];
		obj.Configure(Sprite::ObjectMode::Normal, Sprite::GfxMode::Normal, Sprite::ColorMode::Palette256, Sprite::Shape::square8x8);
		obj.SetNonAffineTransform(false, false, Sprite::Shape::square8x8);
		obj.setPos(8*i, 0);
	}
	const uint8_t data[kNumDigits] = {};
	text.writeNumbers(data, m_DigitSprites, kNumDigits);
}

void FrameCounter::render(TextSystem& text)
{
	// Separate digits
	auto fps = count();
	uint8_t fps100 = uint8_t(fps/100);
	fps -= 100*fps100;
	uint8_t fps10 = (fps/10);
	fps -= fps10 * 10;

	uint8_t counter[kNumDigits] = {fps100, fps10, uint8_t(fps)};

	// Draw frame rate indicator
	text.writeNumbers(counter, m_DigitSprites, kNumDigits);

	// Copy over to VRAM
	for(int i = 0; i < kNumDigits; ++i)
	{
		m_sprites[i] = m_DigitSprites[i];
	}
}

uint32_t FrameCounter::count()
{
#ifdef GBA
	//uint32_t ms = Timer0().counter/16; // ~Milliseconds
	uint32_t tc = Timer0().counter;
	Timer0().reset<Timer::e1024>(); // Reset timer and set prescaler to ~1/16th of a millisecond
	uint32_t fps =tc / 16;// 60;
	//if(tc > 16*16+3*4) // ~16.75, crude approx for 16.6667
	//{
	//	fps = 30;
	//	if(tc > (33*16+8)) // 33.5 ms
	//	{
	//		fps = 20;
	//		if(tc > 16*50) // 50 ms
	//		{
	//			fps=15;
	//			if(tc > 66*16+3*4) // 66.75 ~= 66.6667 ms
	//			{
	//				fps = 10;
	//				if(fps > 100*256) // 100 ms
	//				{
	//					fps = 0; // Slower than 10fps, don't bother
	//				}
	//			}
	//		}
	//	}
	//}
	return fps;
#else
	return 0;
#endif
}