#pragma once

#include "Device.h"
#ifdef _WIN32
#include <Display.h> // For glfw window access
#endif

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
		return s_curState & key;
	}

	inline static bool Pressed(uint16_t key)
	{
		auto lastHeld = s_lastState & key;
		return Held(key) && !lastHeld;
	}

	inline static void Update()
	{
		s_lastState = s_curState;
#ifdef GBA
		s_curState = ~IO::KEYINPUT::Get().value;
#else
		auto window = Mode5Display::s_window;
		// Same key mapping as in the emulators
		updateKey(window, GLFW_KEY_Z, A);
		updateKey(window, GLFW_KEY_X, B);
		updateKey(window, GLFW_KEY_A, L);
		updateKey(window, GLFW_KEY_S, R);
		updateKey(window, GLFW_KEY_DOWN, DOWN);
		updateKey(window, GLFW_KEY_UP, UP);
		updateKey(window, GLFW_KEY_LEFT, LEFT);
		updateKey(window, GLFW_KEY_RIGHT, RIGHT);
		updateKey(window, GLFW_KEY_ENTER, START);
		updateKey(window, GLFW_KEY_BACKSLASH, SELECT);
#endif
	}

#ifdef _WIN32
	inline static void updateKey(GLFWwindow* window, uint32_t glfwKey, uint16_t gbaKey)
	{
		if (glfwGetKey(window, glfwKey) == GLFW_PRESS)
		{
			s_curState |= gbaKey;
		}
		else if (glfwGetKey(window, glfwKey) == GLFW_RELEASE)
		{
			s_curState &= ~gbaKey;
		}
	}
#endif

	static inline int32_t s_lastState = 0;
	static inline int32_t s_curState = 0;
};