#pragma once

#include <cstdint>
#include <Timer.h>
#include <gfx/sprite.h>

class TextSystem;

struct FrameCounter
{
public:
	FrameCounter(TextSystem&);
	void render(TextSystem&);

private:
	uint32_t count();

	static inline constexpr uint32_t kNumDigits = 3;

	Sprite::Object m_DigitSprites[kNumDigits] = {};
	volatile Sprite::Object* m_sprites = nullptr;
};