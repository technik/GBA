#pragma once

#include <linearMath.h>
#include <vector.h>
#include <numbers>

#ifdef GBA
extern "C" {
	#include <tonc.h>
}
#endif // GBA

struct Pose
{
	math::Vec3p16 pos;
	math::unorm16 phi{}; // Rotation around the z axis (normalized to 1 revolution).

	// Cached state
	math::intp12 sinf = math::intp12(0);
	math::intp12 cosf = math::intp12(1);

	void update()
	{
		cosf = math::intp12::castFromShiftedInteger<12>(lu_cos(phi.raw));
		sinf = math::intp12::castFromShiftedInteger<12>(lu_sin(phi.raw));
	}
};

class FPSController
{
public:
	FPSController(Pose& target)
		: m_pose(target)
	{}

	void update();

	Pose& m_pose;

	// Speed controls
	math::intp8 horSpeed = math::intp8(0.06125f);
	math::intp8 verSpeed = math::intp8(0.06125f);
	math::intp16 angSpeed = math::intp16(0.5f);
};

class CharacterController
{
public:
	CharacterController(Pose& target)
		: m_pose(target)
	{}

	void update();

	Pose& m_pose;
	math::intp8 jump = math::intp8(0); // Jump velocity

	// Speed controls
	math::intp16 horSpeed = math::intp16(0.06125f);
	math::intp16 angSpeed = math::intp16(0.5f);
};

struct PoseFollower
{
	PoseFollower(Pose& target, math::Vec3p16 offset)
		: m_target(target)
		, m_offset(offset)
	{
		m_pose.phi = m_target.phi;
		m_pose.pos.z = m_target.pos.z + m_offset.z;
		
		m_pose.pos.x = m_target.pos.x + (m_offset.x * m_pose.cosf - m_offset.y * m_pose.sinf).cast<16>();
		m_pose.pos.y = m_target.pos.x + (m_offset.y * m_pose.cosf + m_offset.x * m_pose.sinf).cast<16>();
	}

	void update()
	{
		m_pose.phi = m_target.phi;
		m_pose.pos.z = m_target.pos.z + m_offset.z;
		
		m_pose.pos.x = m_target.pos.x + (m_offset.x * m_pose.cosf - m_offset.y * m_pose.sinf).cast<16>();
		m_pose.pos.y = m_target.pos.y + (m_offset.y * m_pose.cosf + m_offset.x * m_pose.sinf).cast<16>();

		m_pose.update();
	}

	Pose& m_target;
	Pose m_pose;
	math::Vec3p16 m_offset;
};