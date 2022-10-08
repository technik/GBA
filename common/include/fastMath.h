#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <cmath>

#include "base.h"

#include "linearMath.h"
#include "mercuryLUT.h"

namespace math
{
	inline math::intp16 Sin(math::unorm16 tau)
	{
		int16_t x = (tau.raw + (1 << 5)) >> 6;
		math::intp16 result;
		result.raw = SinP9LUT[x] << 2;
		return result;
	}

	inline math::intp16 Cos(math::unorm16 tau)
	{
		int16_t x = (tau.raw + (1 << 5)) >> 6;
		math::intp16 result;
		result.raw = CosP9LUT[x] << 2;
		return result;
	}

	inline math::intp16 Cotan(math::unorm16 tau)
	{
		int16_t x = (tau.raw + (1 << 4)) >> 5;
		dbgAssert(x < 3 * 0x100 && x > 0x100);
		x -= 0x100;
		math::intp16 result;
		result.raw = CotanP9LUT[x] << 2;
		return result;
	}
}