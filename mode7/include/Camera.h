#pragma once

extern "C" {
#include <tonc.h>
}

#include <demo.h>
#include <Keypad.h>

// Game camera
struct Camera
{
	Camera(VECTOR startPos)
		: pos(startPos)
	{}

	void update()
	{
		VECTOR dir;
		// left/right : strafe
		dir.x = horSpeed * (Keypad::Held(Keypad::R) - Keypad::Held(Keypad::L));
		// up/down : forward/back
		dir.y = horSpeed * (Keypad::Held(Keypad::DOWN) - Keypad::Held(Keypad::UP));
		// B/A : rise/sink
		dir.z = verSpeed*(Keypad::Held(Keypad::B) - Keypad::Held(Keypad::A));

		pos.x += dir.x * cosf.raw - dir.y * sinf.raw;
		pos.y += dir.x * sinf.raw + dir.y * cosf.raw;
		pos.z += dir.z;

		// Limit z to reasonable values to not break the math
		pos.z = max(0, min(250*256, pos.z));

		phi += angSpeed*(Keypad::Held(Keypad::RIGHT) - Keypad::Held(Keypad::LEFT));

		cosf = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(lu_cos(phi));
		sinf = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(lu_sin(phi));
	}

	void postGlobalState()
	{
		// Copy local state into global variables that can be accessed by the renderer
		gCosf = cosf;
		gSinf = sinf;
		gCamPos = pos;
	}

	VECTOR pos;
	FIXED phi = 0;
	math::Fixed<int32_t, 8> sinf = math::Fixed<int32_t, 8>(0);
	math::Fixed<int32_t, 8> cosf = math::Fixed<int32_t, 8>(1);

	FIXED horSpeed = 1;
	FIXED verSpeed = 64;
	FIXED angSpeed = 128;
};