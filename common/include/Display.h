#pragma once

#include <cstdint>
#include "vector.h"
#include "Device.h"

// Config display
constexpr uint32_t ScreenWidth = 240;
constexpr uint32_t ScreenHeight = 160;

class DisplayControl final
{
public:
	// Singleton access
    static DisplayControl& Get() { return *reinterpret_cast<DisplayControl*>(IO::DISPCNT::address); }
	DisplayControl() = delete; // Prevent instantiation

	void InitMode5()
	{
		set<5,BG2>();
		bg2RotScale.a = (160<<8)/ScreenWidth; // =(160/240.0)<<8
		bg2RotScale.d = (128<<8)/ScreenHeight; // =(128/160.0)<<8
	}

	void InitMode2()
	{
		// Enable mode 2 with backgrounds 2 and 3
		set<2,BG2>();
	}

	void StartBlank()
	{
		control |= ForceBlank;
	}

	void EndBlank()
	{
		control &= ~ForceBlank;
	}

	// Display control bits
	static constexpr uint16_t FrameSelect = 1<<4;
	static constexpr uint16_t ForceBlank = 1<<7;
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
		control = videoMode | bgMode;
	};

	void enableSprites()
	{
		control |= (1<<12);
	}

    void flipFrame()
    {
        control ^= FrameSelect;
    }

    uint16_t* backBuffer() const
	{
        return reinterpret_cast<uint16_t*>((control & FrameSelect) ? 0x06000000 : (0x06000000 + 0xA000));
    }

	void vSync()
	{
		while(vCount <= ScreenHeight)
		{}
	}

private:
	struct BGRotScale
	{
		// 16 bit signed quoefficients
		int16_t a;
		int16_t b;
		int16_t c;
		int16_t d;

		// 28 bit signed offsets
		int32_t x0;
		int32_t y0;
	};

	// Register definition
	volatile uint16_t control;
	volatile uint16_t greenSwap;
	volatile uint16_t status;
	volatile uint16_t vCount;
	volatile uint16_t bgControl[4];
	volatile math::Vec2<uint16_t> bgScroll[4];
	volatile BGRotScale bg2RotScale;
	volatile BGRotScale bg3RotScale;
};

inline auto& Display() { return DisplayControl::Get(); }