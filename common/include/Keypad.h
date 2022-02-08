#pragma once

#include "Device.h"

struct Keypad
{
	Keypad() = delete;
	Keypad(const Keypad&) = delete;

	static constexpr uint16_t KEY_A = 1<<0;
	static constexpr uint16_t KEY_B = 1<<1;
	static constexpr uint16_t KEY_SELECT = 1<<2;
	static constexpr uint16_t KEY_START = 1<<3;
	static constexpr uint16_t KEY_RIGHT = 1<<4;
	static constexpr uint16_t KEY_LEFT = 1<<5;
	static constexpr uint16_t KEY_UP = 1<<6;
	static constexpr uint16_t KEY_DOWN = 1<<7;
	static constexpr uint16_t KEY_R = 1<<8;
	static constexpr uint16_t KEY_L = 1<<9;

	static bool Held(uint16_t key)
	{
		return !(IO::KEYINPUT::Get().value & key);
	}
};