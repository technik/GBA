#pragma once
#include <linearMath.h>
#include <vector.h>
#include <Display.h>

// Game camera
struct Camera
{
	Camera(math::Vec3p8 startPos)
		: pos(startPos)
	{}

	void update()
	{
#ifdef GBA
		math::Vec3p8 dir;
		// left/right : strafe
		dir.x() = horSpeed * (Keypad::Held(Keypad::R) - Keypad::Held(Keypad::L));
		// up/down : forward/back
		dir.y() = horSpeed * (Keypad::Held(Keypad::DOWN) - Keypad::Held(Keypad::UP));
		// B/A : rise/sink
		dir.z() = verSpeed*(Keypad::Held(Keypad::B) - Keypad::Held(Keypad::A));

		pos.x() += (dir.x() * cosf + dir.y() * sinf).cast<8>();
		pos.y() += (dir.y() * cosf - dir.x() * sinf).cast<8>();
		pos.z() += dir.z();

		// Limit z to reasonable values to not break the math
		pos.z() = math::max(math::intp8(0), min(math::intp8(250), pos.z()));

		phi += angSpeed*(Keypad::Held(Keypad::LEFT) - Keypad::Held(Keypad::RIGHT));

		cosf = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(lu_cos(phi.raw));
		sinf = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(lu_sin(phi.raw));
#endif
	}

	// Returns x,y in screen space and depth as z
	math::Vec3p8 projectWorldPos(math::Vec3p8 worldPos) const
	{
		math::Vec3p8 viewSpace;
		viewSpace.y() = pos.z() - worldPos.z();
		math::Vec2p8 relHorPos = math::Vec2p8(worldPos.x() - pos.x(), worldPos.y() - pos.y());
		viewSpace.x() = (relHorPos.x() * cosf + relHorPos.y() * sinf).cast<8>();
		viewSpace.z() = (relHorPos.y() * cosf - relHorPos.x() * sinf).cast<8>();
		math::intp8 invDepth = math::intp8(1) / viewSpace.z();
		math::Vec3p8 result;
		result.z() = viewSpace.z();
		result.x() = (viewSpace.x() * invDepth).cast<8>() * ScreenHeight + (ScreenWidth/2);
		result.y() = (viewSpace.y() * invDepth).cast<8>() * ScreenHeight + (ScreenHeight/2);
		return result;
	}

	math::Vec3p8 pos;
	math::intp8 phi {};
	math::intp8 sinf = math::intp8(0);
	math::intp8 cosf = math::intp8(1);

	math::intp8 horSpeed = math::intp8(0.5f);
	math::intp8 verSpeed = math::intp8(0.25f);
	math::intp8 angSpeed = math::intp8(0.5f);
};