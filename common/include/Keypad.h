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

	inline static int32_t Held(uint16_t key)
	{
		return int32_t(!(IO::KEYINPUT::Get().value & key));
	}
};