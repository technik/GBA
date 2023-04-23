#pragma once

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <cstdint>
#include <cstddef>
#include "linearMath.h"

namespace math {

	template<class T, size_t N>
	struct Vector;

	template<class T> using Vec2 = Vector<T, 2>;
	template<class T> using Vec3 = Vector<T, 3>;
	template<class T> using Vec4 = Vector<T, 4>;

	template<class T>
	struct Vector<T,2> final
	{
		T x;
		T y;

		// Constructors
		Vector() = default;
		constexpr explicit Vector(T _x) : x(_x), y(_x)
		{
		}

		constexpr Vector(T _x, T _y) : x(_x),y(_y)
		{
		}

		// Assignment and copy construction
		constexpr Vector(const Vec2<T>& other) : x(other.x), y(other.y)
		{
		}

		auto& operator=(const Vec2<T>& other)
		{
			x = other.x;
			y = other.y;
			return *this;
		}
		
		auto& operator=(const Vec2<T>& other) volatile
		{
			x = other.x;
			y = other.y;
			return *this;
		}

		T& operator()(int i) { return (&x)[i]; }
		const T& operator()(int i) const { return (&x)[i]; }
	};

	template<class T>
	struct Vector<T,3> final
	{
		T x;
		T y;
		T z;

		// Constructors
		Vector() = default;
		constexpr explicit Vector(T _x)
		{
			x = _x;
			y = _x;
			z = _x;
		}
		constexpr Vector(T _x, T _y, T _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		// Assignment and copy construction
		constexpr Vector(const Vec3<T>& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}

		auto& operator=(const Vec3<T>& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}
		
		auto& operator=(const Vec3<T>& other) volatile
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		T& operator()(int i) { return (&x)[i]; }
		const T& operator()(int i) const { return (&x)[i]; }
	};

	template<class T>
	struct Vector<T, 4> final
	{
		T x;
		T y;
		T z;
		T w;

		// Constructors
		Vector() = default;
		constexpr explicit Vector(T _x)
		{
			x = _x;
			y = _x;
			z = _x;
		}
		constexpr Vector(T _x, T _y, T _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}

		// Assignment and copy construction
		constexpr Vector(const Vec3<T>& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}

		auto& operator=(const Vec3<T>& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		auto& operator=(const Vec3<T>& other) volatile
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		T& operator()(int i) { return (&x)[i]; }
		const T& operator()(int i) const { return (&x)[i]; }
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
		return Vec2<T>(a.x + b.x, a.y + b.y);
	}

	template<class T>
	inline Vec2<T>& operator+=(Vec2<T>& a, const Vec2<T>& b)
	{
		a = Vec2<T>(a.x + b.x, a.y + b.y);
		return a;
	}

	template<class T>
	constexpr inline Vec2<T> operator-(const Vec2<T>& a, const Vec2<T>& b)
	{
		return Vec2<T>(a.x - b.x, a.y - b.y);
	}

	template<class T>
	inline Vec2<T>& operator-=(Vec2<T>& a, const Vec2<T>& b)
	{
		a = Vec2<T>(a.x - b.x, a.y - b.y);
		return a;
	}

	template<class T>
	constexpr inline auto dot(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x*b.x + a.y*b.y;
	}

	template<class T>
	constexpr inline auto cross(const Vec2<T>& a, const Vec2<T>&b)
	{
		return a.x*b.y - a.y*b.x;
	}

	template<>
	constexpr inline auto cross(const Vec2p8& a, const Vec2p8& b)
	{
		// Pre shifting to avoid overflow
		return a.x.round<4>() * b.y.round<4>() - a.y.round<4>() * b.x.round<4>();
	}

	template<class T, class K>
	constexpr inline auto operator*(const Vec2<T>& v, K x)
	{
		using retT = decltype(v.x*x);
		return Vec2<retT>{v.x*x, v.y*x};
	}
	
	// 3d vector operators
	template<class T>
	constexpr inline Vec3<T> operator+(const Vec3<T>& a, const Vec3<T>& b)
	{
		return {a.x + b.x, a.y + b.y, a.z + b.z };
	}

	template<class T>
	inline Vec3<T> operator+=(Vec3<T>& a, const Vec3<T>& b)
	{
		return a = {a.x + b.x, a.y + b.y, a.z + b.z };
	}

	template<class T>
	constexpr inline Vec3<T> operator-(const Vec3<T>& a, const Vec3<T>& b)
	{
		return {a.x - b.x, a.y - b.y, a.z - b.z };
	}

	template<class T>
	inline Vec3<T> operator-=(Vec3<T>& a, const Vec3<T>& b)
	{
		return a = {a.x - b.x, a.y - b.y, a.z - b.z };
	}

	template<class T>
	constexpr inline auto dot(const Vec3<T>& a, const Vec3<T>&b)
	{
		return a.x*b.x + a.y*b.y +a.z*b.z ;
	}

} // namespace math

#endif // _VECTOR_H_