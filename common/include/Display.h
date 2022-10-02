#pragma once

#ifndef GBA
#include "imageUtils.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>

#include <glad.h>
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif
#endif // _WIN32

#include <cstdint>
#include "vector.h"
#include "Device.h"
#include "Color.h"

// Config display
constexpr int32_t ScreenWidth = 240;
constexpr int32_t ScreenHeight = 160;

class DisplayControl final
{
public:
	// Singleton access
    static DisplayControl& Get() { return *reinterpret_cast<DisplayControl*>(IO::DISPCNT::address); }
	DisplayControl() = delete; // Prevent instantiation

	void InitMode5()
	{
#ifndef _WIN32
		SetMode<5,BG2>();
		bg2RotScale.a = (160<<8)/ScreenWidth; // =(160/240.0)<<8
		bg2RotScale.d = (128<<8)/ScreenHeight; // =(128/160.0)<<8
#endif
	}

	auto& BG2RotScale() { return bg2RotScale; }

	void StartBlank()
	{
#ifdef GBA
		control = control | ForceBlank;
#endif
	}

	void EndBlank()
	{
#ifndef _WIN32
		control = control & (~ForceBlank);
#endif
	}

	void EnableHBlankAccess()
	{
#ifndef _WIN32
		control = control | HBlank;
#endif
	}

	void DisableHBlankAccess()
	{
#ifndef _WIN32
		control = control & (~HBlank);
#endif
	}

	// Display control bits
	static constexpr uint16_t FrameSelect = 1<<4;
	static constexpr uint16_t HBlank = 1<<5;
	static constexpr uint16_t ForceBlank = 1<<7;
	static constexpr uint16_t BG0 = 1<<8;
	static constexpr uint16_t BG1 = 1<<9;
	static constexpr uint16_t BG2 = 1<<10;
	static constexpr uint16_t BG3 = 1<<11;
	static constexpr uint16_t OBJ = 1<<12;

	template<uint16_t videoMode, uint16_t bgMode>
	void SetMode()
	{
#ifndef _WIN32
		static_assert(videoMode < 6, "Only video modes 0-5 are enabled in the GBA");
        static_assert(bgMode <= (BG0+BG1+BG2+BG3) && bgMode >= BG0);
		control = videoMode | bgMode;
#endif
	};

	void enableSprites()
	{
#ifndef _WIN32
		control = control | (1<<12) | (1<<6);
#endif
	}

    void flipFrame()
    {
#ifndef _WIN32
        control = control ^ FrameSelect;
#endif
    }

    uint16_t* backBuffer() const
	{
#ifndef _WIN32
        return reinterpret_cast<uint16_t*>((control & FrameSelect) ? 0x06000000 : (0x06000000 + 0xA000));
#else
		return nullptr;
#endif
    }

	void vSync()
	{
#ifndef _WIN32
		while(vCount > ScreenHeight)
		{}
		while(vCount <= ScreenHeight)
		{}
#endif
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

class Mode3Display
{
public:
	static constexpr uint32_t Width = 240;
	static constexpr uint32_t Height = 160;

	void Init()
	{
		auto& disp = DisplayControl::Get();
		disp.SetMode<3,DisplayControl::BG2>();
	}

	// There is no backbuffer in Mode3!
	static Color* frontBuffer()
	{
		return reinterpret_cast<Color*>(0x06000000);
	}
};

class Mode4Display
{
public:
	static constexpr uint32_t Width = 240;
	static constexpr uint32_t Height = 160;

	void flip()
	{
		DisplayControl::Get().flipFrame();
	}

	void Init()
	{
		auto& disp = DisplayControl::Get();
		disp.SetMode<4,DisplayControl::BG2>();
	}

	volatile uint16_t* backBuffer()
	{
		auto& disp = DisplayControl::Get();
		return disp.backBuffer();
	}
};

class Mode5Display
{
public:
	static constexpr uint32_t Width = 160;
	static constexpr uint32_t Height = 128;
	static constexpr uint32_t Area = Width*Height;
	static constexpr uint32_t BufferSize = Width*Height * 2;

	static inline uint32_t pixel(uint32_t x, uint32_t y)
	{
		dbgAssert(x < Width&& y < Height);
		return x + Width * y;
	}

	bool BeginFrame();
	void Flip();

	bool Init();

	static Color* backBuffer()
	{
#ifdef _WIN32
		return s_backBuffer.data();
#else
		auto& disp = DisplayControl::Get();
		return reinterpret_cast<Color*>(disp.backBuffer());
#endif
	}

#ifdef _WIN32
	inline static GLFWwindow* s_window;
	uint32_t m_backBufferTexture;
	inline static std::vector<Color> s_backBuffer;

	uint32_t m_VBO;
	uint32_t m_VAO;
	uint32_t m_fullScreenShader = uint32_t(-1);
#endif // _WIN32
};