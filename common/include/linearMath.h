#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <cmath>

#include <base.h>

#ifdef _WIN32
#include <numbers>

// Mock GBA lib functions
inline int sgn(int x)
{
	return (x >= 0) ? +1 : -1;
}

inline int sgn3(int x)
{
	return (x >> 31) - (-x >> 31);
}

inline int max(int a, int b)
{
	return (a > b) ? (a) : (b);
}

//! Get the minimum of \a a and \a b
inline int min(int a, int b)
{
	return (a < b) ? (a) : (b);
}

inline int16_t ArcTan(int16_t x)
{
	float arg = float(x) / (1 << 14);
	float rev = float(atan(arg) / (2 * std::numbers::pi));
	return int16_t(8192 * 8 * rev);
}

//! Look-up a sine value (2&#960; = 0x10000)
/*! \param theta Angle in [0,FFFFh] range
*	 \return .12f sine value
*/
inline int32_t lu_sin(uint32_t theta)
{
	int16_t t = (theta>>7)&0x1ff; // Emulate the LUT loss of precision
	float radians = float(t) / (1<<9) * (2 * std::numbers::pi); // Transform to radians
	return std::floor(sin(radians) * (1<<12));
}

//! Look-up a cosine value (2&#960; = 0x10000)
/*! \param theta Angle in [0,FFFFh] range
*	 \return .12f cosine value
*/
inline int32_t lu_cos(uint32_t theta)
{
	return lu_sin(theta + (1 << 14));
}

#endif // _WIN32

namespace math
{
	template<class T>
	FORCE_INLINE constexpr auto max(T a, T b)
	{	
		return a < b ? b : a;
	}

	template<class T>
	FORCE_INLINE constexpr auto min(T a, T b)
	{	
		return a < b ? a : b;
	}

	template<class T>
	FORCE_INLINE constexpr auto saturate(T x)
	{
		constexpr auto one = T(1);
		constexpr auto zero = T(0);
		
		return max(zero, min(x, one));
	}

	template<std::integral Store, size_t Shift>
	class Fixed
	{
	public:
		using store_type = Store;
		static constexpr size_t shift = Shift;

		store_type raw;

		FORCE_INLINE Fixed() = default;
		template<std::integral T>
		FORCE_INLINE constexpr explicit Fixed(T x)
		{
			raw = Store(x << shift);
			//dbgAssert((raw >> shift) == x);
		}
		constexpr explicit Fixed(float x) : raw(Store(x* (1 << shift))) {}
		FORCE_INLINE constexpr Fixed(const math::Fixed<Store, Shift>& _other)
		{
			raw = _other.raw;
		}

		FORCE_INLINE constexpr store_type floor() const { return raw >> shift; }

		FORCE_INLINE explicit operator float() const { return float(raw) / (1 << shift); }

		// Cast without rounding
		template<size_t otherShift, std::integral otherStore>
		static constexpr Fixed castFromShiftedInteger(otherStore x)
		{
			Fixed result;
			if constexpr (otherShift > shift) // the other type shift is bigger than this, need to shift it right
			{
				constexpr size_t diff = otherShift - shift;
				result.raw = x / (1 << diff);
			}
			else
			{
				constexpr size_t diff = shift - otherShift;
				result.raw = x * (1 << diff);
				dbgAssert((result.raw >> diff) == x);
			}
			return result;
		}

		// Cast with rounding
		template<size_t otherShift, std::unsigned_integral otherStore>
		FORCE_INLINE static constexpr Fixed roundFromShiftedInteger(otherStore x)
		{
			Fixed result;
			if constexpr (otherShift > shift) // the other type shift is bigger than this, need to shift it right and round
			{
				constexpr size_t diff = otherShift - shift;
				constexpr size_t half = 1 << (diff - 1);
				result.raw = (x + half) / (1 << diff);
			}
			else
			{
				// No need to round anything when upcasting
				constexpr size_t diff = shift - otherShift;
				result.raw = x * (1 << diff);
				dbgAssert((result.raw >> diff) == x);
			}
			return result;
		}

		// Performs a destructive cast without rounding
		template<size_t resultShift, class resultStore = store_type>
		FORCE_INLINE constexpr auto cast() const
		{
			Fixed<resultStore, resultShift> result;
			if constexpr (resultShift > shift) // the other type shift is bigger than this, shift left
			{
				constexpr size_t diff = resultShift - shift;
				result.raw = raw * (1 << diff);
				dbgAssert((result.raw >> diff) == raw); // Overflow!
			}
			else
			{
				constexpr size_t diff = shift - resultShift;
				result.raw = raw / (1 << diff);
			}
			return result;
		}

		// Performs a destructive cast with rounding
		template<size_t resultShift>
		constexpr auto round() const
		{
			Fixed<store_type, resultShift> result;
			if constexpr (resultShift >= shift) // the other type shift is bigger than this, shift left
			{
				constexpr size_t diff = resultShift - shift;
				result.raw = raw * (1 << diff);
			}
			else
			{
				if constexpr (std::is_signed_v<store_type>)
				{
					// Dividing, need to round
					constexpr size_t diff = shift - resultShift;
					size_t half = raw >= 0 ? (1 << (diff - 1)) : -(1 << (diff - 1));
					result.raw = (raw + half) / (1 << diff);
				}
				else
				{
					// Dividing, need to round
					constexpr size_t diff = shift - resultShift;
					constexpr size_t half = 1 << (diff - 1);
					result.raw = (raw + half) / (1 << diff);
				}
			}
			return result;
		}

		// Performs a destructive cast with rounding
		constexpr auto roundToInt() const
		{
			store_type result;
			if constexpr (std::is_signed_v<store_type>) // More expensive implementation for signed
			{
				// Dividing, need to round
				store_type half = raw >= 0 ? (1 << (shift - 1)) : -(1 << (shift - 1));
				result = (raw + half) / (1 << shift);
			}
			else
			{
				// Dividing, need to round
				constexpr size_t half = 1 << (shift - 1);
				result = (raw + half) / (1 << shift);
			}
			return result;
		}

		template<size_t resultShift>
		constexpr auto roundUnsafe() const // The caller is responsible for making sure the value is positive
		{
			dbgAssert(raw >= 0);
			Fixed<store_type, resultShift> result;
			if constexpr (resultShift >= shift) // the other type shift is bigger than this, shift left
			{
				constexpr store_type diff = resultShift - shift;
				result.raw = raw * (1 << diff);
			}
			else
			{
				// Dividing, need to round
				constexpr store_type diff = shift - resultShift;
				constexpr store_type half = 1 << (diff - 1);
				result.raw = (raw + half) / (1 << diff);
			}
			return result;
		}

		template<std::integral T>
		FORCE_INLINE void operator+=(T x)
		{
			const auto shifted = x * (1 << shift);
			assert(shifted / (1 << shift) == x); // Overflow!
			raw += shifted;
		}

		FORCE_INLINE void operator+=(Fixed x)
		{
			raw += x.raw;
		}

		template<std::integral T>
		FORCE_INLINE void operator-=(T x)
		{
			const auto shifted = x * (1 << shift);
			assert(shifted / (1 << shift) == x); // Overflow!
			raw -= shifted;
		}

		FORCE_INLINE void operator-=(Fixed x)
		{
			raw -= x.raw;
		}
	};

	// Unorm16 specialization
	template<>
	class Fixed<uint16_t,16>
	{
	public:
		using store_type = uint16_t;
		static constexpr size_t shift = 16;

		store_type raw;
		
		FORCE_INLINE Fixed() = default;
		template<std::integral T>
		FORCE_INLINE constexpr explicit Fixed(T x)
		{
			raw = uint16_t(x<<shift);
			//dbgAssert((raw >> shift) == x);
		}
		constexpr explicit Fixed(float x) : raw(uint16_t(x * (1<<shift))) {}
		FORCE_INLINE constexpr Fixed(const math::Fixed<uint16_t,shift>& _other)
		{
			raw = _other.raw;
		}

		FORCE_INLINE explicit operator float() const { return float(raw) / (1<<shift); }

		// Cast without rounding
		template<size_t otherShift, std::integral otherStore>
		static constexpr Fixed castFromShiftedInteger(otherStore x)
		{
			Fixed result;
			if constexpr(otherShift > shift) // the other type shift is bigger than this, need to shift it right
			{
				constexpr size_t diff = otherShift - shift;
				result.raw = x / (1<<diff);
			}
			else
			{
				constexpr size_t diff = shift - otherShift;
				result.raw = x * (1<<diff);
			}
			return result;
		}

		// Performs a destructive cast without rounding
		template<size_t resultShift, class resultStore = store_type>
		FORCE_INLINE constexpr auto cast() const
		{
			Fixed<resultStore, resultShift> result;
			if constexpr(resultShift > shift) // the other type shift is bigger than this, shift left
			{
				constexpr size_t diff = resultShift - shift;
				result.raw = raw * (1<<diff);
				dbgAssert((result.raw >> diff) == raw); // Overflow!
			}
			else
			{
				constexpr size_t diff = shift - resultShift;
				result.raw = raw / (1<<diff);
			}
			return result;
		}

		// Performs a destructive cast with rounding
		template<size_t resultShift>
		constexpr auto round() const
		{
			Fixed<store_type, resultShift> result;
			if constexpr(resultShift >= shift) // the other type shift is bigger than this, shift left
			{
				constexpr store_type diff = resultShift - shift;
				result.raw = raw * (1<<diff);
			}
			else
			{
				// Dividing, need to round
				constexpr store_type diff = shift - resultShift;
				constexpr store_type half = 1<<(diff-1);
				result.raw = (raw+half) / (1<<diff);
			}
			return result;
		}

		template<std::integral T>
		FORCE_INLINE void operator+=(T x)
		{
			const auto shifted = x * (1 << shift);
			raw += shifted;
		}
		
		FORCE_INLINE void operator+=(Fixed x)
		{
			raw += x.raw;
		}

		template<std::integral T>
		FORCE_INLINE void operator-=(T x)
		{
			const auto shifted = x * (1 << shift);
			raw -= shifted;
		}
		
		FORCE_INLINE void operator-=(Fixed x)
		{
			raw -= x.raw;
		}
	};

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr auto operator+(
		const Fixed<StoreA, shift>& a,
		const Fixed<StoreB, shift>& b)
	{
		using sum_type = decltype(a.raw + b.raw);
		Fixed<sum_type, shift> result;
		result.raw = a.raw + b.raw;
		return result;
	}

	FORCE_INLINE constexpr auto operator+(
		const Fixed<uint16_t, 16>& a,
		const Fixed<uint16_t, 16>& b)
	{
		Fixed<uint16_t, 16> result;
		result.raw = a.raw + b.raw;
		return result;
	}

	template<class Store, size_t shift>
	FORCE_INLINE constexpr auto operator+(
		const Fixed<Store, shift>& a,
		std::integral auto b)
	{
		Fixed<Store, shift> result;
		result.raw = a.raw + (b<<shift);
		return result;
	}

	template<class Store, size_t shift>
	FORCE_INLINE constexpr auto operator+(
		std::integral auto a,
		const Fixed<Store, shift>& b)
	{
		Fixed<Store, shift> result;
		result.raw = (a<<shift) + b.raw;
		return result;
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr auto operator-(
		const Fixed<StoreA, shift>& a,
		const Fixed<StoreB, shift>& b)
	{
		using sub_type = decltype(a.raw - b.raw);
		Fixed<sub_type, shift> result;
		result.raw = a.raw - b.raw;
		return result;
	}

	FORCE_INLINE constexpr auto operator-(
		const Fixed<uint16_t, 16>& a,
		const Fixed<uint16_t, 16>& b)
	{
		Fixed<uint16_t, 16> result;
		result.raw = uint16_t(a.raw - b.raw);
		return result;
	}

	template<std::integral Store, size_t shift>
	FORCE_INLINE constexpr auto operator-(
		const Fixed<Store, shift>& a)
	{
		static_assert(std::is_signed_v<Store>, "Can't flip the sign on an unsigned type");

		Fixed<Store, shift> result;
		result.raw = -a.raw;
		return result;
	}

	template<class Store, size_t shift>
	FORCE_INLINE constexpr auto operator-(
		const Fixed<Store, shift>& a,
		std::integral auto b)
	{
		Fixed<Store, shift> result;
		result.raw = a.raw - (b<<shift);
		return result;
	}

	template<class Store, size_t shift>
	FORCE_INLINE constexpr auto operator-(
		std::integral auto a,
		const Fixed<Store, shift>& b)
	{
		Fixed<Store, shift> result;
		result.raw = (a<<shift) - b.raw;
		return result;
	}

	template<std::integral Store, size_t shift>
	FORCE_INLINE constexpr auto abs(
		const Fixed<Store, shift>& a)
	{
		Fixed<Store, shift> result;
		result.raw = std::abs(a.raw);
		return result;
	}

	template<
		class StoreA, size_t shiftA,
		class StoreB, size_t shiftB>
	FORCE_INLINE constexpr auto operator*(
		const Fixed<StoreA, shiftA>& a,
		const Fixed<StoreB, shiftB>& b)
	{
		using product_type = decltype(a.raw*b.raw);
		Fixed<product_type, shiftA+shiftB> result;
		result.raw = product_type(int64_t(a.raw)*int64_t(b.raw));
		dbgAssert(int64_t(a.raw) * b.raw == int64_t(result.raw)); // Overflow detected!
		return result;
	}

	// Explicit override for 16.16
	FORCE_INLINE inline auto operator*(const Fixed<int32_t,16>& a, const Fixed<int32_t, 16>& b)
	{
		Fixed<int32_t, 16> result;
		result.raw = int32_t((int64_t(a.raw) * int64_t(b.raw)) >> 16);
		dbgAssert((int64_t(a.raw) * b.raw)>>16 == int64_t(result.raw)); // Overflow detected!
		return result;
	}

	template<
		class StoreA, size_t shiftA,
		std::integral T>
	FORCE_INLINE constexpr auto operator*(
		const Fixed<StoreA, shiftA>& a,
		T b)
	{
		using product_type = decltype(a.raw*b);
		Fixed<product_type, shiftA> result;
		result.raw = a.raw*b;
		return result;
	}

	template<std::integral T>
	FORCE_INLINE constexpr auto operator*(
		const Fixed<int32_t, 16>& a,
		T b)
	{
		Fixed<int32_t, 16> result;
		result.raw = a.raw * b;
		return result;
	}

    template<
        class StoreA, size_t shiftA,
        std::integral T>
	FORCE_INLINE constexpr auto operator*(
            T b,
            const Fixed<StoreA, shiftA>& a)
    {
        using product_type = decltype(a.raw* b);
        Fixed<product_type, shiftA> result;
        result.raw = a.raw * b;
        return result;
    }

	template<
		class StoreA,
		class StoreB,
        size_t shift>
	FORCE_INLINE constexpr auto operator / (
		const Fixed<StoreA, shift>& a,
		const Fixed<StoreB, shift>& b)
	{
		using div_type = decltype(a.raw / b.raw);
		Fixed<div_type, shift> result;
		result.raw = (int64_t(a.raw)<<shift) / b.raw;
		return result;
	}

	template<
		class StoreA, size_t shiftA,
		std::integral T>
	FORCE_INLINE constexpr auto operator/(
		Fixed<StoreA, shiftA> a,
		T b)
	{
		using div_type = decltype(a.raw / b);
		Fixed<div_type, shiftA> result;
		result.raw = a.raw / b;
		return result;
	}

	template<class StoreA, size_t shiftA, std::integral T>
	FORCE_INLINE constexpr bool operator== (Fixed<StoreA, shiftA> a, T b)
	{
		return a.raw == (b<<shiftA);
	}

	template<class StoreA, size_t shiftA, std::integral T>
	FORCE_INLINE constexpr bool operator!= (Fixed<StoreA, shiftA> a, T b)
	{
		return a.raw != (b<<shiftA);
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr bool operator== (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw == b.raw;
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr bool operator!= (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw != b.raw;
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr bool operator< (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw < b.raw;
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr bool operator<= (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw <= b.raw;
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr bool operator> (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw > b.raw;
	}

	template<class StoreA, class StoreB, size_t shift>
	FORCE_INLINE constexpr bool operator>= (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw >= b.raw;
	}

	template<class StoreA, size_t shift>
	FORCE_INLINE constexpr bool operator< (Fixed<StoreA, shift> a, int b)
	{
		return a.raw < (b<<shift);
	}

	template<class StoreA, size_t shift>
	FORCE_INLINE constexpr bool operator<= (Fixed<StoreA, shift> a, int b)
	{
		return a.raw <= (b<<shift);
	}

	template<class StoreA, size_t shift>
	FORCE_INLINE constexpr bool operator> (Fixed<StoreA, shift> a, int b)
	{
		return a.raw > (b<<shift);
	}

	template<class StoreA, size_t shift>
	FORCE_INLINE constexpr bool operator>= (Fixed<StoreA, shift> a, int b)
	{
		return a.raw >= (b<<shift);
	}

	template<class Store, size_t shift>
	FORCE_INLINE constexpr auto sqrt(Fixed<Store,shift> x)
	{
		static_assert((shift & 1) == 0);
		constexpr auto resultShift = shift/2;
		Fixed<Store,resultShift> result;
		result.raw = ::sqrt(x.raw);
		return result;
	}

	using intp8 = Fixed<int32_t,8>;
	using int8p8 = Fixed<int16_t,8>;
	using intp12 = Fixed<int32_t,12>;
	using intp16 = Fixed<int32_t,16>;
	using intp24 = Fixed<int32_t,24>;
	using unorm16 = Fixed<uint16_t, 16>;

	CONSTEVAL intp8 operator""_p8(long double x) { return intp8(float(x)); }
	CONSTEVAL intp8 operator""_p8(unsigned long long x) { return intp8(long(x)); }
	CONSTEVAL intp12 operator""_p12(long double x) { return intp12(float(x)); }
	CONSTEVAL intp12 operator""_p12(unsigned long long x) { return intp12(long(x)); }
	CONSTEVAL intp16 operator""_p16(long double x) { return intp16(float(x)); }
	CONSTEVAL intp16 operator""_p16(unsigned long long x) { return intp16(long(x)); }
	CONSTEVAL unorm16 operator""_u16(long double x) { return unorm16(float(x)); }
	CONSTEVAL unorm16 operator""_u16(unsigned long long x) { return unorm16(long(x)); }
	CONSTEVAL intp24 operator""_p24(long double x) { return intp24(float(x)); }
	CONSTEVAL intp24 operator""_p24(unsigned long long x) { return intp24(long(x)); }

} // namespace math