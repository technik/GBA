#pragma once

#include <cstdint>
#include <cstddef>
#include "linearMath.h"
#include <vector.h>

namespace math {

	template<class T, uint32_t M, uint32_t N>
	struct Matrix final
	{
		static constexpr uint32_t numRows = M;
		static constexpr uint32_t numCols = N;
		static constexpr uint32_t numElements = M*N;

		T m[M][N];

		// Constructors
		T& operator()(uint32_t i, uint32_t j) { return m[i][j]; }
		const T& operator()(uint32_t i, uint32_t j) const { return m[i][j]; }

		static constexpr Matrix Zero()
		{
			Matrix result {};
			return result;
		}
		
		static constexpr Matrix Identity()
		{
			Matrix result {};
			for(int32_t i = 0; i < min(numRows,numCols); ++i)
				result.m[i][i] = 1;
			return result;
		}

		auto& operator=(const Matrix<T,M,N>& other)
		{
			for(uint32_t i = 0; i < M; ++i)
			{
				for(uint32_t j = 0; j < N; ++j)
				{
					m[i][j] = other.m[i][j];
				}
			}
			return *this;
		}
		
		auto& operator=(const Matrix<T,M,N>& other) volatile
		{
			for(uint32_t i = 0; i < M; ++i)
			{
				for(uint32_t j = 0; j < N; ++j)
				{
					m[i][j] = other.m[i][j];
				}
			}
			return *this;
		}
	};

	// Affine transforms as expected by the hardware registers
	struct AffineTransform2D
	{
		int16_t a,b,c,d;
		Vec2p8 x, y;
	};

	template<class T>
	using Mat22 = Matrix<T,2,2>;
	template<class T>
	using Mat33 = Matrix<T,3,3>;
	template<class T>
	using Mat34 = Matrix<T, 3, 4>;
	template<class T>
	using Mat44 = Matrix<T, 4, 4>;

	using Mat22f = Mat22<float>;
	using Mat33f = Mat33<float>;
	
	using Mat22p8 = Mat22<intp8>;
	using Mat33p8 = Mat33<intp8>;
	
	using Mat22p12 = Mat22<intp12>;
	using Mat33p12 = Mat33<intp12>;
	
	using Mat22p16 = Mat22<intp16>;
	using Mat33p16 = Mat33<intp16>;
	using Mat34p16 = Mat34<intp16>;
	using Mat44p16 = Mat44<intp16>;

	// Maps depth from 0 (z=-inf) to 1 (z=-n)
	inline Mat44p16 projectionMatrix(
		intp16 xFocalLength, // yFocalLength / aspectRatio
		intp16 yFocalLength, // 1 / std::tan(yFovRad / 2)
		intp16 n) // Near clip
	{
		// Precomputations
		auto B = 2 * n;
		// P * (0,0,-n,1) = (0,0,-n,-n) = (0,0,n,n)
		// P.z/w = 1;
		// P * (0,0,-i,1) = (0,0,-n,-i); z/w = 0
		return {
			xFocalLength,  0_p16, 0_p16, 0_p16,
			0_p16, -yFocalLength, 0_p16, 0_p16,
			0_p16,         0_p16, 0_p16,     n,
			0_p16,         0_p16,-1_p16, 0_p16
			};
	}

	// Assumes 4th row of b is (0,0,0,1)
	inline Mat44p16 operator*(const Mat44p16& a, const Mat34p16& b)
	{
		Mat44p16 result;
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				result(i, j) =
					a(i, 0) * b(0, j) +
					a(i, 1) * b(1, j) +
					a(i, 2) * b(2, j);
			}
			result(i, 3) =
				a(i, 0) * b(0, 3) +
				a(i, 1) * b(1, 3) +
				a(i, 2) * b(2, 3) +
				a(i, 3);
		}
		return result;
	}

	// Assumes 4th row of a and b is (0,0,0,1)
	inline Mat34p16 operator*(const Mat34p16& a, const Mat34p16& b)
	{
		Mat34p16 result;
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				result(i, j) =
					a(i, 0) * b(0, j) +
					a(i, 1) * b(1, j) +
					a(i, 2) * b(2, j);
			}
			result(i, 3) =
				a(i, 0) * b(0, 3) +
				a(i, 1) * b(1, 3) +
				a(i, 2) * b(2, 3) +
				a(i, 3);
		}
		return result;
	}

	inline Vec3p16 projectPosition(const Mat44p16& proj, const Vec3p16& pos)
	{
		Vec3p16 result;
		for (int i = 0; i < 3; ++i)
		{
			result(i) =
				proj(i, 0) * pos.x +
				proj(i, 1) * pos.y +
				proj(i, 2) * pos.z +
				proj(i, 3);
		}
		intp16 w =
			proj(3, 0) * pos.x +
			proj(3, 1) * pos.y +
			proj(3, 2) * pos.z +
			proj(3, 3);
		return { result.x / w, result.y / w, result.z / w };
	}

	// Returns a position into clip space, already divided by the homogeneous coordinate.
	// Assumes:
	// - positive pos.x is right
	// - positive pos.y is forward
	// - positive pos.z is up
	// Returns a new position in clip space where
	// - positive x is to the right [-1,1], with 0 at the center of the screen
	// - positive y is downward [-1,1], with 0 at the center of the screen
	// - positive z is into the screen [0,1]
	// Other assumptions:
	// - Aspect ratio 3:2
	// - Horizontal fov = 2*atan(0.5)
	// - Near clip: 1/128
	// - Far clip: 256+near
	inline Vec3p8 projectPosition(const Vec3p8& pos)
	{
		Vec3p8 result;
		// TODO: InvDepth
		result.x = (pos.x * 2) / pos.y;
		result.y = -(pos.z * 3) / pos.y;
		result.z = result.y / 256;
		return result;
	}

} // namespace math
