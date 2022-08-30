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
		constexpr explicit Vector(T _x)
		{
			for(size_t i = 0; i < N; ++i)
				m[i] = _x;
		}
		constexpr Vector(T _x, T _y) : m{_x,_y} {}
		constexpr Vector(T _x, T _y, T _z) : m{_x,_y,_z} {}

		// Assignment and copy construction
		constexpr Vector(const Vector<T,N>& other)
		{
			for(size_t i = 0; i < N; ++i)
				m[i] = other.m[i];
		}

		auto& operator=(const Vector<T,N>& other)
		{
			for(size_t i = 0; i < N; ++i)
				m[i] = other.m[i];
			return *this;
		}
		
		auto& operator=(const Vector<T,N>& other) volatile
		{
			for(size_t i = 0; i < N; ++i)
				m[i] = other.m[i];
			return *this;
		}

		// Accessors
		FORCE_INLINE  auto x() const { return m[0]; }
		FORCE_INLINE  auto y() const { static_assert(N>1); return m[1]; }
		FORCE_INLINE  auto z() const { static_assert(N>2); return m[2]; }
		
		FORCE_INLINE  auto& x() { return m[0]; }
		FORCE_INLINE  auto& y() { static_assert(N>1); return m[1]; }
		FORCE_INLINE  auto& z() { static_assert(N>2); return m[2]; }
		FORCE_INLINE  volatile auto& x() volatile { return m[0]; }
		FORCE_INLINE  volatile auto& y() volatile { static_assert(N>1); return m[1]; }
		FORCE_INLINE  volatile auto& z() volatile { static_assert(N>2); return m[2]; }
	};

	template<class T>
	using Vec2 = Vector<T,2>;
	template<class T>
	using Vec3 = Vector<T,3>;

	using Vec2f = Vector<float,2>;
	using Vec3f = Vector<float,3>;

	using Vec2i = Vector<int32_t,2>;
	using Vec3i = Vector<int32_t,3>;

	using Vec2u = Vector<uint32_t,2>;
	using Vec3u = Vector<uint32_t,3>;

	using Vec2p8 = Vec2<intp8>;
	using Vec3p8 = Vec3<intp8>;
	
	using Vec2p12 = Vec2<intp12>;
	using Vec3p12 = Vec3<intp12>;
	
	using Vec2p16 = Vec2<intp16>;
	using Vec3p16 = Vec3<intp16>;

	using Vec2p24 = Vec2<intp24>;
	using Vec3p24 = Vec3<intp24>;

	// Basic operators
	template<class T>
	constexpr inline Vec2<T> operator+(const Vec2<T>& a, const Vec2<T>& b)
	{
		return {a.x() + b.x(), a.y() + b.y()};
	}

	template<class T>
	inline Vec2<T> operator+=(Vec2<T>& a, const Vec2<T>& b)
	{
		return a = {a.x() + b.x(), a.y() + b.y()};
	}

	template<class T>
	constexpr inline Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b)
	{
		return {a.x() - b.x(), a.y() - b.y()};
	}

	template<class T>
	inline Vec2<T> operator-=(Vec2<T>& a, const Vec2<T>& b)
	{
		return a = {a.x() - b.x(), a.y() - b.y()};
	}

	template<class T>
	constexpr inline auto dot(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x()*b.x() + a.y()*b.y();
	}

	template<class T>
	constexpr inline auto cross(const Vec2<T>& a, const Vec2<T>&b)
	{
		return (a.x()*b.y() - a.y()*b.x());
	}

	template<class T, class K>
	constexpr inline auto operator*(const Vec2<T>& v, K x)
	{
		using retT = decltype(v.x()*x);
		return Vec2<retT>{v.x()*x, v.y()*x};
	}
	
	// 3d vector operators
	template<class T>
	constexpr inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b)
	{
		return {a.x() + b.x(), a.y() + b.y(), a.z() + b.z() };
	}

	template<class T>
	inline Vec3<T> operator+=(Vec3<T>& a, const Vec3<T>& b)
	{
		return a = {a.x() + b.x(), a.y() + b.y(), a.z() + b.z() };
	}

	template<class T>
	constexpr inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b)
	{
		return {a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
	}

	template<class T>
	inline Vec3<T> operator-=(Vec3<T>& a, const Vec3<T>& b)
	{
		return a = {a.x() - b.x(), a.y() - b.y(), a.z() - b.z() };
	}

	template<class T>
	constexpr inline auto dot(const Vec3<T>& a, const Vec3<T>&b)
	{
		return a.x()*b.x() + a.y()*b.y() +a.z()*b.z() ;
	}

} // namespace math

#endif // _VECTOR_H_