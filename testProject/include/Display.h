#pragma once

#include <cstdint>

struct DisplayControl
{
	uint16_t reg;

    static DisplayControl& Get() { return *reinterpret_cast<DisplayControl*>(0x04000000); }

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

    uint16_t* backBuffer() const{
        return reinterpret_cast<uint16_t*>((reg & FrameSelect) ? 0x06000000 : (0x06000000 + 0xA000));
    }
};