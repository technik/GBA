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
		Matrix() = default;

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

		// Assignment and copy construction
		constexpr Matrix(const Matrix<T,M,N>& other)
		{
			for(uint32_t i = 0; i < M; ++i)
			{
				for(uint32_t j = 0; j < N; ++j)
				{
					m[i][j] = other.m[i][j];
				}
			}
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

	using Mat22f = Mat22<float>;
	using Mat33f = Mat33<float>;
	
	using Mat22p8 = Mat22<intp8>;
	using Mat33p8 = Mat33<intp8>;
	
	using Mat22p12 = Mat22<intp12>;
	using Mat33p12 = Mat33<intp12>;
	
	using Mat22p16 = Mat22<intp16>;
	using Mat33p16 = Mat33<intp16>;

} // namespace math
