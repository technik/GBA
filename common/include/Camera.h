#pragma once

#include <Display.h>
#include <linearMath.h>
#include <numbers>
#include <pose.h>
#include <vector.h>
#include <matrix.h>

// Game camera
struct Camera
{
	// Camera coordinates:
	// x points to the right of the screen,
	// y points forward
	// z points upwards
	Camera(int32_t clipWidth, int32_t clipHeight, math::Vec3p16 startPos)
		: m_halfClipWidth(clipWidth / 2)
		, m_halfClipHeight(clipHeight / 2)
		, m_pose{ startPos, math::unorm16(0) }
	{}

	// Returns x,y in screen space and depth as z
	math::Vec3p16 projectWorldPos(math::Vec3p16 worldPos) const
	{
		// Transform position to camera space
		math::Vec3p16 viewSpace;
		viewSpace.z = m_pose.pos.z - worldPos.z; // invert vertical sign because screen space y points downwards
		math::Vec2p16 relHorPos = math::Vec2p16(worldPos.x - m_pose.pos.x, worldPos.y - m_pose.pos.y);
		viewSpace.x = (relHorPos.x * m_pose.cosf + relHorPos.y * m_pose.sinf).cast<16>();
		viewSpace.y = (relHorPos.y * m_pose.cosf - relHorPos.x * m_pose.sinf).cast<16>();
		// Project x,y onto the screen
		// invDepth is actually tg(fov_y/2) / depth
		math::intp16 invDepth = viewSpace.y.raw ? math::intp16(2) / viewSpace.y : math::intp16(0);
		math::Vec3p16 result;
		result.x = (viewSpace.x * invDepth).cast<16>() * m_halfClipHeight + m_halfClipWidth;
		result.y = (viewSpace.z * invDepth).cast<16>() * m_halfClipHeight + m_halfClipHeight;
		result.z = viewSpace.y; // Return linear depth
		return result;
	}

	// Returns x in screen space, depth as y and invDepth as z
	math::Vec3p8 projectWorldPos2D(const math::Vec2p16& worldPos) const
	{
		// Transform position to camera space
		auto camX = m_pose.pos.x;
		auto camY = m_pose.pos.y;
		math::Vec2p16 relPos;
		relPos.x = worldPos.x - camX;
		relPos.y = worldPos.y - camY;
		math::Vec2p12 viewSpace;
		viewSpace.x = (relPos.x * m_pose.cosf - relPos.y * m_pose.sinf).cast<12>();
		viewSpace.y = (relPos.y * m_pose.cosf + relPos.x * m_pose.sinf).cast<12>();
		// Project x onto the screen
		math::intp12 invDepth = viewSpace.y.raw ? math::intp12(1) / viewSpace.y : math::intp12(0);
		math::Vec3p8 result;
		result.x = (viewSpace.x * invDepth + 1).cast<8>() * m_halfClipWidth;
		result.y = viewSpace.y.cast<8>();
		result.z = invDepth.cast<8>();
		return result;
	}

	int32_t m_halfClipWidth, m_halfClipHeight;
	Pose m_pose;
};

class YawPitchCamera
{
public:
	math::Vec3p8 pos{};
	math::unorm16 yaw{};
	math::unorm16 pitch{};

	math::intp12 yaw_cosf;
	math::intp12 yaw_sinf;
	math::intp12 pit_cosf;
	math::intp12 pit_sinf;

	math::Mat33p12 rotMtx;

	void refreshRot()
	{
		yaw_cosf = math::intp12::castFromShiftedInteger<12>(lu_cos(yaw.raw));
		yaw_sinf = math::intp12::castFromShiftedInteger<12>(lu_sin(yaw.raw));
		pit_cosf = math::intp12::castFromShiftedInteger<12>(lu_cos(pitch.raw));
		pit_sinf = math::intp12::castFromShiftedInteger<12>(lu_sin(pitch.raw));

		rotMtx(0, 0) = yaw_cosf;
		rotMtx(1, 0) = -(yaw_sinf * pit_cosf).cast<12>();
		rotMtx(2, 0) = (yaw_sinf * pit_sinf).cast<12>();
		rotMtx(0, 1) = yaw_sinf;
		rotMtx(1, 1) = (yaw_cosf * pit_cosf).cast<12>();
		rotMtx(2, 1) = -(yaw_cosf * pit_sinf).cast<12>();
		rotMtx(0, 2) = {};
		rotMtx(1, 2) = pit_sinf;
		rotMtx(2, 2) = pit_cosf;
	}

	//math::Mat34p16 worldView(const math::Mat34p16& worldMtx)
	//{
	//	//Mat34p16 result;
	//}

	math::Vec3p8 transformPos(const math::Vec3p8 x) const
	{
		math::Vec3p8 relPos = x - pos;
		return transformDir(relPos);
	}

	math::Vec3p8 transformDir(const math::Vec3p8 x) const
	{
		math::Vec3p8 result;
		// Precomputed
		result(0) = (rotMtx(0, 0) * x(0) + rotMtx(0, 1) * x(1)).cast<8>();
		result(1) = (rotMtx(1, 0) * x(0) + rotMtx(1, 1) * x(1) + rotMtx(1, 2) * x(2)).cast<8>();
		result(2) = (rotMtx(2, 0) * x(0) + rotMtx(2, 1) * x(1) + rotMtx(2, 2) * x(2)).cast<8>();
		return result;
	}

	void update(const math::intp16& horSpeed, const math::intp16& andSpeed);
};