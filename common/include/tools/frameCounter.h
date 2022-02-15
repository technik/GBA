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
	Sprite::Object& tile(uint32_t n);

	const uint32_t m_spriteNdx;
};