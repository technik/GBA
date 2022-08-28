#pragma once

#include <Display.h>
#include <linearMath.h>
#include <numbers>
#include <pose.h>

// Game camera
struct Camera
{
	// Camera coordinates:
	// x points to the right of the screen,
	// y points forward
	// z points upwards
	Camera(int32_t clipWidth, int32_t clipHeight, math::Vec3p8 startPos)
		: m_halfClipWidth(clipWidth / 2)
		, m_halfClipHeight(clipHeight / 2)
		, m_pose{ startPos, math::intp16(0) }
	{}

	// Returns x,y in screen space and depth as z
	math::Vec3p8 projectWorldPos(math::Vec3p8 worldPos) const
	{
		// Transform position to camera space
		math::Vec3p8 viewSpace;
		viewSpace.z() = m_pose.pos.z() - worldPos.z(); // invert vertical sign because screen space y points downwards
		math::Vec2p8 relHorPos = math::Vec2p8(worldPos.x() - m_pose.pos.x(), worldPos.y() - m_pose.pos.y());
		viewSpace.x() = (relHorPos.x() * m_pose.cosf + relHorPos.y() * m_pose.sinf).cast<8>();
		viewSpace.y() = (relHorPos.y() * m_pose.cosf - relHorPos.x() * m_pose.sinf).cast<8>();
		// Project x,y onto the screen
		// invDepth is actually tg(fov_y/2) / depth
		math::intp8 invDepth = viewSpace.y().raw ? math::intp8(2) / viewSpace.y() : math::intp8(0);
		math::Vec3p8 result;
		result.x() = (viewSpace.x() * invDepth).cast<8>() * m_halfClipHeight + m_halfClipWidth;
		result.y() = (viewSpace.z() * invDepth).cast<8>() * m_halfClipHeight + m_halfClipHeight;
		result.z() = viewSpace.y(); // Return linear depth
		return result;
	}

	int32_t m_halfClipWidth, m_halfClipHeight;
	Pose m_pose;
};