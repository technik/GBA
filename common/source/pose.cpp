
#include <pose.h>
#include <Keypad.h>

using namespace math;

void FPSController::update()
{
	Vec3p8 dir;
	// left/right : strafe
	dir.x() = horSpeed * (Keypad::Held(Keypad::R) - Keypad::Held(Keypad::L));
	// up/down : forward/back
	dir.y() = horSpeed * (Keypad::Held(Keypad::UP) - Keypad::Held(Keypad::DOWN));
	// B/A : rise/sink
	dir.z() = verSpeed*(Keypad::Held(Keypad::A) - Keypad::Held(Keypad::B));

	m_pose.pos.x() += (dir.x() * m_pose.cosf - dir.y() * m_pose.sinf).cast<8>();
	m_pose.pos.y() += (dir.x() * m_pose.sinf + dir.y() * m_pose.cosf).cast<8>();
	m_pose.pos.z() += dir.z();

	// Limit z to reasonable values to not break the math
	m_pose.pos.z() = max(intp8(0.5), min(intp8(25), m_pose.pos.z()));

	m_pose.phi += (angSpeed*(Keypad::Held(Keypad::LEFT) - Keypad::Held(Keypad::RIGHT))).cast<16>();

	m_pose.update();
}

void CharacterController::update()
{
	Vec3p8 dir = {};
	// left/right : strafe/rotate
	if(Keypad::Held(Keypad::L))
	{
		dir.x() = horSpeed * (Keypad::Held(Keypad::RIGHT) - Keypad::Held(Keypad::LEFT));
	}
	else
	{
		m_pose.phi += (angSpeed*(Keypad::Held(Keypad::LEFT) - Keypad::Held(Keypad::RIGHT))).cast<16>();
	}
	// up/down : forward/back
	dir.y() = horSpeed * (Keypad::Held(Keypad::UP) - Keypad::Held(Keypad::DOWN));
	dir.z() = horSpeed * (Keypad::Held(Keypad::A) - Keypad::Held(Keypad::B));

	Vec2p8 disp;
	disp.x() = (dir.x() * m_pose.cosf - dir.y() * m_pose.sinf).cast<8>();
	disp.y() = (dir.y() * m_pose.cosf + dir.x() * m_pose.sinf).cast<8>();

	m_pose.pos.x() += disp.x();
	m_pose.pos.y() += disp.y();
	m_pose.pos.z() += dir.z();


	// Jumps
	/*
	if(Keypad::Pressed(Keypad::A) && m_pose.pos.z() == 0_p8)
	{
		jump = 7_p8; // Impulse
	}

	if(m_pose.pos.z() > 0_p8 || jump > 0_p8)
	{
		// Integrate jump
		jump -= 10_p8 / 32; // Impulse
		m_pose.pos.z() += jump / 32; // speed/~fps

		// Reset state on contact
		if(m_pose.pos.z() < 0_p8)
		{
			m_pose.pos.z() = 0_p8;
			jump = 0_p8;
		}
	}*/

	m_pose.update();
}