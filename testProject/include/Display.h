#pragma once

#include <cstdint>
#include "vector.h"

// Config display
#define USE_VIDEO_MODE_5
#ifdef USE_VIDEO_MODE_3
	constexpr uint32_t ScreenWidth = 240;
	constexpr uint32_t ScreenHeight = 160;
	constexpr uint16_t VideoMode = 3;
#else // VIDEO MODE 5
	constexpr uint32_t ScreenWidth = 160;
	constexpr uint32_t ScreenHeight = 128;
	constexpr uint16_t VideoMode = 5;
#endif

class DisplayControl
{
public:
	// Singleton access
    static DisplayControl& Get() { return *reinterpret_cast<DisplayControl*>(0x04000000); }
	DisplayControl() = delete; // Prevent instantiation

	// Display control bits
	static constexpr uint16_t FrameSelect = 1<<4;
	static constexpr uint16_t BG0 = 1<<8;
	static constexpr uint16_t BG1 = 1<<9;
	static constexpr uint16_t BG2 = 1<<10;
	static constexpr uint16_t BG3 = 1<<11;
	static constexpr uint16_t OBJ = 1<<12;

	template<uint16_t videoMode, uint16_t bgMode>
	void set()
	{
		static_assert(videoMode < 6, "Only video modes 0-5 are enabled in the GBA");
        static_assert(bgMode <= (BG0+BG1+BG2+BG3) && bgMode >= BG0);
		reg = videoMode | bgMode;
	};

    void flipFrame()
    {
        reg ^= FrameSelect;
    }

    uint16_t* backBuffer() const
	{
        return reinterpret_cast<uint16_t*>((reg & FrameSelect) ? 0x06000000 : (0x06000000 + 0xA000));
    }

	void vSync()
	{
		while(vCount != (ScreenHeight+1))
		{}
	}

private:
	struct BGRotScale
	{
		uint16_t dx;
		uint16_t dmx;
		uint16_t dy;
		uint16_t dmy;
		uint32_t x0;
		uint32_t y0;
	};

	// Register definition
	volatile uint16_t reg;
	uint16_t padding0;
	volatile uint16_t status;
	volatile uint16_t vCount;
	volatile uint16_t bgControl[4];
	volatile Vec2<uint16_t> bgOffset[3];
	volatile BGRotScale bg2RotScale;
	volatile BGRotScale bg3RotScale;
};

inline auto& Display() { return DisplayControl::Get(); }