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

		pos.x += dir.x * cosf - dir.y * sinf;
		pos.y += dir.x * sinf + dir.y * cosf;
		pos.z += dir.z;

		// Limit z to reasonable values to not break the math
		pos.z = max(0, min(250*256, pos.z));

		phi += angSpeed*(Keypad::Held(Keypad::RIGHT) - Keypad::Held(Keypad::LEFT));

		cosf = (lu_cos(phi)+(1<<3))/(1<<4);
		sinf = (lu_sin(phi)+(1<<3))/(1<<4);
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
	FIXED sinf = 0;
	FIXED cosf = 1<<8;

	FIXED horSpeed = 1;
	FIXED verSpeed = 64;
	FIXED angSpeed = 128;
};