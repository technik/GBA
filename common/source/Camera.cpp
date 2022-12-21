#include "Camera.h"
#include "Keypad.h"

using namespace math;

void YawPitchCamera::update(const math::intp16& horSpeed, const math::intp16& angSpeed)
{
	math::Vec2p12 dir = {};
	// left/right : strafe/rotate
	if (Keypad::Held(Keypad::L))
	{
		dir.x = (horSpeed * (Keypad::Held(Keypad::RIGHT) - Keypad::Held(Keypad::LEFT))).cast<12>();
	}
	else
	{
		yaw += unorm16::castFromShiftedInteger<16>((angSpeed * (Keypad::Held(Keypad::LEFT) - Keypad::Held(Keypad::RIGHT))).raw);
	}
	// up/down : forward/back
	dir.y = (horSpeed * (Keypad::Held(Keypad::UP) - Keypad::Held(Keypad::DOWN))).cast<12>();

	Vec2p16 disp;
	disp.x = (dir.x * yaw_cosf - dir.y * yaw_sinf).cast<16>();
	disp.y = (dir.y * yaw_cosf + dir.x * yaw_sinf).cast<16>();

	pos.x += disp.x.cast<8>();
	pos.y += disp.y.cast<8>();
	pos.z += (horSpeed * (Keypad::Held(Keypad::A) - Keypad::Held(Keypad::B))).cast<8>();
}