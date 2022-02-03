#pragma once

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <cstddef>
#include "linearMath.h"

namespace math {

	template<class T, size_t N>
	struct Vector final
	{
		T m[N];

		// Constructors
		Vector() = default;
		Vector(T _x)
		{
			for(size_t i = 0; i < N; ++i)
				m[i] = _x;
		}
		Vector(T _x, T _y) : m{_x,_y} {}
		Vector(T _x, T _y, T _z) : m{_x,_y,_z} {}

		// Assignment and copy construction
		Vector(const Vector& other)
		{
			for(size_t i = 0; i < N; ++i)
				m[i] = other.m[i];
		}

		// Accessors
		auto& x() const { return m[0]; }
		auto& y() const { static_assert(N>1); return m[1]; }
		auto& z() const { static_assert(N>2); return m[2]; }
		
		auto& x() { return m[0]; }
		auto& y() { static_assert(N>1); return m[1]; }
		auto& z() { static_assert(N>2); return m[2]; }
	};

	template<class T>
	using Vec2 = Vector<T,2>;
	template<class T>
	using Vec3 = Vector<T,3>;

	using Vec2f = Vector<float,2>;
	using Vec3f = Vector<float,3>;

	using Vec2p8 = Vec2<intp8>;
	using Vec3p8 = Vec3<intp8>;

	// Basic operators
	template<class T>
	inline Vec2<T> operator+(const Vec2<T>& a, const Vec2<T>& b)
	{
		return {a.x() + b.x(), a.y() + b.y()};
	}

	template<class T>
	inline Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b)
	{
		return {a.x() - b.x(), a.y() - b.y()};
	}

	template<class T>
	inline T dot(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x()*b.x() + a.y()*b.y();
	}

	template<class T>
	inline T cross(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x()*b.y() - a.y()*b.x();
	}

} // namespace math

#endif // _VECTOR_H_