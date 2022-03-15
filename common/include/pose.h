#pragma once

#include <linearMath.h>
#include <vector.h>

struct Pose
{
	math::Vec3p8 pos;
	math::intp8 phi{}; // Rotation around the z axis (normalized to 1 revolution).

	// Cached state
	math::intp8 sinf = math::intp8(0);
	math::intp8 cosf = math::intp8(1);

	void update()
	{
#ifdef GBA
		cosf = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(lu_cos(phi.raw));
		sinf = math::Fixed<int32_t, 8>::castFromShiftedInteger<12>(lu_sin(phi.raw));
#else
		float phiFloat = phi.raw * 2 * std::numbers::pi_v<float> / float(1 << 8);
		cosf = math::intp8((float)cos(phiFloat));
		sinf = math::intp8((float)sin(phiFloat));
#endif
	}
};