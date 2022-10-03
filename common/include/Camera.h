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
		viewSpace.z() = m_pose.pos.z() - worldPos.z(); // invert vertical sign because screen space y points downwards
		math::Vec2p16 relHorPos = math::Vec2p16(worldPos.x() - m_pose.pos.x(), worldPos.y() - m_pose.pos.y());
		viewSpace.x() = (relHorPos.x() * m_pose.cosf + relHorPos.y() * m_pose.sinf).cast<16>();
		viewSpace.y() = (relHorPos.y() * m_pose.cosf - relHorPos.x() * m_pose.sinf).cast<16>();
		// Project x,y onto the screen
		// invDepth is actually tg(fov_y/2) / depth
		math::intp16 invDepth = viewSpace.y().raw ? math::intp16(2) / viewSpace.y() : math::intp16(0);
		math::Vec3p16 result;
		result.x() = (viewSpace.x() * invDepth).cast<16>() * m_halfClipHeight + m_halfClipWidth;
		result.y() = (viewSpace.z() * invDepth).cast<16>() * m_halfClipHeight + m_halfClipHeight;
		result.z() = viewSpace.y(); // Return linear depth
		return result;
	}

	// Returns x in screen space, depth as y and invDepth as z
	math::Vec3p8 projectWorldPos2D(const math::Vec2p16& worldPos) const
	{
		// Transform position to camera space
		auto camX = m_pose.pos.x();
		auto camY = m_pose.pos.y();
		math::Vec2p16 relPos;
		relPos.x() = worldPos.x() - camX;
		relPos.y() = worldPos.y() - camY;
		math::Vec2p12 viewSpace;
		viewSpace.x() = (relPos.x() * m_pose.cosf + relPos.y() * m_pose.sinf).cast<12>();
		viewSpace.y() = (relPos.y() * m_pose.cosf - relPos.x() * m_pose.sinf).cast<12>();
		// Project x onto the screen
		math::intp12 invDepth = viewSpace.y().raw ? math::intp12(1) / viewSpace.y() : math::intp12(0);
		math::Vec3p8 result;
		result.x() = (viewSpace.x() * invDepth + 1).cast<8>() * m_halfClipWidth;
		result.y() = viewSpace.y().cast<8>();
		result.z() = invDepth.cast<8>();
		return result;
	}

	int32_t m_halfClipWidth, m_halfClipHeight;
	Pose m_pose;
};