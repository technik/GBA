#pragma once

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <cstddef>
#include "linearMath.h"

namespace math {

	template<class T>
	struct Vec2 final
	{
		T m_x;
		T m_y;

		// Constructors
		Vec2() = default;
		constexpr explicit Vec2(T _x) : m_x(_x), m_y(_x)
		{
		}

		constexpr Vec2(T _x, T _y) : m_x(_x), m_y(_y)
		{
		}

		// Assignment and copy construction
		constexpr Vec2(const Vec2<T>& other) : m_x(other.m_x), m_y(other.m_y)
		{
		}

		auto& operator=(const Vec2<T>& other)
		{
			m_x = other.m_x;
			m_y = other.m_y;
			return *this;
		}
		
		auto& operator=(const Vec2<T>& other) volatile
		{
			m_x = other.m_x;
			m_y = other.m_y;
			return *this;
		}

		// Accessors
		FORCE_INLINE  auto& x() const { return m_x; }
		FORCE_INLINE  auto& y() const { return m_y; }
		
		FORCE_INLINE  auto& x() { return m_x; }
		FORCE_INLINE  auto& y() { return m_y; }
	};

	template<class T>
	struct Vec3 final
	{
		T m_x;
		T m_y;
		T m_z;

		// Constructors
		Vec3() = default;
		constexpr explicit Vec3(T _x)
		{
			m_x = _x;
			m_y = _x;
			m_z = _x;
		}
		constexpr Vec3(T _x, T _y, T _z)
		{
			m_x = _x;
			m_y = _y;
			m_z = _z;
		}

		// Assignment and copy construction
		constexpr Vec3(const Vec3<T>& other)
		{
			m_x = other.m_x;
			m_y = other.m_y;
			m_z = other.m_z;
		}

		auto& operator=(const Vec3<T>& other)
		{
			m_x = other.m_x;
			m_y = other.m_y;
			m_z = other.m_z;
			return *this;
		}
		
		auto& operator=(const Vec3<T>& other) volatile
		{
			m_x = other.m_x;
			m_y = other.m_y;
			m_z = other.m_z;
			return *this;
		}

		// Accessors
		const T& x() const { return m_x; }
		const T& y() const { return m_y; }
		const T& z() const { return m_z; }
		
		T& x() { return m_x; }
		T& y() { return m_y; }
		T& z() { return m_z; }
	};

	using Vec2f = Vec2<float>;
	using Vec3f = Vec3<float>;

	using Vec2i = Vec2<int32_t>;
	using Vec3i = Vec3<int32_t>;

	using Vec2u = Vec2<uint32_t>;
	using Vec3u = Vec3<uint32_t>;

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
		return Vec2<T>(a.x() + b.x(), a.y() + b.y());
	}

	template<class T>
	inline Vec2<T>& operator+=(Vec2<T>& a, const Vec2<T>& b)
	{
		a = Vec2<T>(a.x() + b.x(), a.y() + b.y());
		return a;
	}

	template<class T>
	constexpr inline Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b)
	{
		return Vec2<T>(a.x() - b.x(), a.y() - b.y());
	}

	template<class T>
	inline Vec2<T>& operator-=(Vec2<T>& a, const Vec2<T>& b)
	{
		a = Vec2<T>(a.x() - b.x(), a.y() - b.y());
		return a;
	}

	template<class T>
	constexpr inline auto dot(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x()*b.x() + a.y()*b.y();
	}

	template<class T>
	constexpr inline auto cross(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x()*b.y() - a.y()*b.x();
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