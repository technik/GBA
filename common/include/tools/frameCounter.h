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

	Sprite::Object m_ShadowSprites[2];
	Sprite::Object* m_sprites = nullptr;
};