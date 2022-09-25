#pragma once

#include "Device.h"

struct Keypad
{
	Keypad() = delete;
	Keypad(const Keypad&) = delete;

	static constexpr uint16_t A = 1<<0;
	static constexpr uint16_t B = 1<<1;
	static constexpr uint16_t SELECT = 1<<2;
	static constexpr uint16_t START = 1<<3;
	static constexpr uint16_t RIGHT = 1<<4;
	static constexpr uint16_t LEFT = 1<<5;
	static constexpr uint16_t UP = 1<<6;
	static constexpr uint16_t DOWN = 1<<7;
	static constexpr uint16_t R = 1<<8;
	static constexpr uint16_t L = 1<<9;

	inline static bool Held(uint16_t key)
	{
#ifdef GBA
		return s_curState & key;
#else
		return false;
#endif
	}

	inline static bool Pressed(uint16_t key)
	{
#ifdef GBA
		auto lastHeld = s_lastState & key;
		return Held(key) && !lastHeld;
#else
		return false;
#endif
	}

	inline static void Update()
	{
#ifdef GBA
		s_lastState = s_curState;
		s_curState = ~IO::KEYINPUT::Get().value;
#endif
	}

	static inline int32_t s_lastState = 0;
	static inline int32_t s_curState = 0;
};