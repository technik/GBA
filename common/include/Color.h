#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>

struct Color
{
	Color() = default;
	
	template<std::integral T>
	constexpr Color(T r, T g, T b)
		: raw((r&0x1f)|((g&0x1f)<<5)|((b&0x1f)<<10))
	{
	}

	constexpr Color(float r, float g, float b)
		: raw(0)
	{
		raw |= uint32_t(r*0x1f) & 0x1f;
		raw |= (uint32_t(g*0x1f) & 0x1f) << 5;
		raw |= (uint32_t(b*0x1f) & 0x1f) << 10;
	}

	uint16_t raw;
};

namespace BasicColor
{
	static inline constexpr Color Black = Color(0,0,0);
	static inline constexpr Color White = Color(0x1f,0x1f,0x1f);
	static inline constexpr Color Red = Color(0x1f,0,0);
	static inline constexpr Color Green = Color(0, 0x1f,0);
	static inline constexpr Color DarkGreen = Color(0, 0x0f,0);
	static inline constexpr Color Blue = Color(0,0,0x1f);
	static inline constexpr Color SkyBlue = Color(0x11,0x16,0x1f);
	static inline constexpr Color Yellow = Color(0x1f,0x1f,0x00);
	static inline constexpr Color Orange = Color(0x1f,0x0f,0x00);
	static inline constexpr Color Pink = Color(0x1f,0x0f,0x0f);
	static inline constexpr Color DarkGrey = Color(0x07,0x07,0x07);
	static inline constexpr Color MidGrey = Color(0x0f,0x0f,0x0f);
	static inline constexpr Color LightGrey = Color(0x17,0x17,0x17);
}