#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>
#include <cmath>

namespace math
{
	template<class T>
	constexpr auto max(T a, T b)
	{	
		return a < b ? b : a;
	}

	template<class T>
	constexpr auto min(T a, T b)
	{	
		return a < b ? a : b;
	}

	template<class T>
	constexpr auto saturate(T x)
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
		
		constexpr Fixed() : raw(0) {}
		template<std::integral T>
		constexpr explicit Fixed(T x) : raw(x<<shift) {}
		constexpr explicit Fixed(float x) : raw(x * (1<<shift)) {}

		constexpr store_type floor() const { return raw >> shift; }

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

		// Cast with rounding
		template<size_t otherShift, std::unsigned_integral otherStore>
		static constexpr Fixed roundFromShiftedInteger(otherStore x)
		{
			Fixed result;
			if constexpr(otherShift > shift) // the other type shift is bigger than this, need to shift it right and round
			{
				constexpr size_t diff = otherShift - shift;
				constexpr size_t half = 1<<(diff-1);
				result.raw = (x+half) / (1<<diff);
			}
			else
			{
				// No need to round anything when upcasting
				constexpr size_t diff = shift - otherShift;
				result.raw = x * (1<<diff);
			}
			return result;
		}

		// Performs a destructive cast without rounding
		template<size_t resultShift, class resultStore = store_type>
		constexpr auto cast() const
		{
			Fixed<resultStore, resultShift> result;
			if constexpr(resultShift > shift) // the other type shift is bigger than this, shift left
			{
				constexpr size_t diff = resultShift - shift;
				result.raw = raw * (1<<diff);
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
				constexpr size_t diff = resultShift - shift;
				result.raw = raw * (1<<diff);
			}
			else
			{
				if constexpr(std::is_signed_v<store_type>)
				{
					// Dividing, need to round
					constexpr size_t diff = shift - resultShift;
					size_t half = raw >= 0 ? (1<<(diff-1)) : -(1<<(diff-1));
					result.raw = (raw+half) / (1<<diff);
				}
				else
				{
					// Dividing, need to round
					constexpr size_t diff = shift - resultShift;
					constexpr size_t half = 1<<(diff-1);
					result.raw = (raw+half) / (1<<diff);
				}
			}
			return result;
		}
		
		// Performs a destructive cast with rounding
		constexpr auto roundToInt() const
		{
			store_type result;
			if constexpr(std::is_signed_v<store_type>) // More expensive implementation for signed
			{
				// Dividing, need to round
				size_t half = raw >= 0 ? (1<<(shift-1)) : -(1<<(shift-1));
				result = (raw+half) / (1<<shift);
			}
			else
			{
				// Dividing, need to round
				constexpr size_t half = 1<<(shift-1);
				result = (raw+half) / (1<<shift);
			}
			return result;
		}

		template<size_t resultShift>
		constexpr auto roundUnsafe() const // The caller is responsible for making sure the value is positive
		{
			Fixed<store_type, resultShift> result;
			if constexpr(resultShift >= shift) // the other type shift is bigger than this, shift left
			{
				constexpr size_t diff = resultShift - shift;
				result.raw = raw * (1<<diff);
			}
			else
			{
				// Dividing, need to round
				constexpr size_t diff = shift - resultShift;
				constexpr size_t half = 1<<(diff-1);
				result.raw = (raw+half) / (1<<diff);
			}
			return result;
		}

		template<std::integral T>
		void operator+=(T x)
		{
			raw += x*(1<<shift);
		}
		
		void operator+=(Fixed x)
		{
			raw += x.raw;
		}

		template<std::integral T>
		void operator-=(T x)
		{
			raw += x*(1<<shift);
		}
		
		void operator-=(Fixed x)
		{
			raw += x.raw;
		}
	};

	template<class StoreA, class StoreB, size_t shift>
	constexpr auto operator+(
		Fixed<StoreA, shift> a,
		Fixed<StoreB, shift> b)
	{
		using sum_type = decltype(a.raw + b.raw);
		Fixed<sum_type, shift> result;
		result.raw = a.raw + b.raw;
		return result;
	}

	template<class Store, size_t shift>
	constexpr auto operator+(
		Fixed<Store, shift> a,
		std::integral auto b)
	{
		Fixed<Store, shift> result;
		result.raw = a.raw + (b<<shift);
		return result;
	}

	template<class StoreA, class StoreB, size_t shift>
	constexpr auto operator-(
		Fixed<StoreA, shift> a,
		Fixed<StoreB, shift> b)
	{
		using sub_type = decltype(a.raw - b.raw);
		Fixed<sub_type, shift> result;
		result.raw = a.raw - b.raw;
		return result;
	}

	template<std::integral Store, size_t shift>
	constexpr auto operator-(
		Fixed<Store, shift> a)
	{
		static_assert(std::is_signed_v<Store>, "Can't flip the sign on an unsigned type");

		Fixed<Store, shift> result;
		result.raw = -a.raw;
		return result;
	}

	template<
		class StoreA, size_t shiftA,
		class StoreB, size_t shiftB>
	constexpr auto operator*(
		Fixed<StoreA, shiftA> a,
		Fixed<StoreB, shiftB> b)
	{
		using product_type = decltype(a.raw*b.raw);
		Fixed<product_type, shiftA+shiftB> result;
		result.raw = a.raw*b.raw;
		return result;
	}

	template<
		class StoreA, size_t shiftA,
		std::integral T>
	constexpr auto operator*(
		Fixed<StoreA, shiftA> a,
		T b)
	{
		using product_type = decltype(a.raw*b);
		Fixed<product_type, shiftA> result;
		result.raw = a.raw*b;
		return result;
	}

    template<
        class StoreA, size_t shiftA,
        std::integral T>
        constexpr auto operator*(
            T b,
            Fixed<StoreA, shiftA> a)
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
	constexpr auto operator / (
		Fixed<StoreA, shift> a,
		Fixed<StoreB, shift> b)
	{
		using div_type = decltype(a.raw / b.raw);
		Fixed<div_type, shift> result;
		result.raw = (a.raw<<shift) / b.raw;
		return result;
	}

	template<
		class StoreA, size_t shiftA,
		std::integral T>
	constexpr auto operator/(
		Fixed<StoreA, shiftA> a,
		T b)
	{
		using div_type = decltype(a.raw / b);
		Fixed<div_type, shiftA> result;
		result.raw = a.raw / b;
		return result;
	}

	template<class StoreA, class StoreB, size_t shift>
	constexpr bool operator< (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw < b.raw;
	}

	template<class StoreA, class StoreB, size_t shift>
	constexpr bool operator> (Fixed<StoreA, shift> a, Fixed<StoreB, shift> b)
	{
		return a.raw > b.raw;
	}

	template<class Store, size_t shift>
	constexpr auto sqrt(Fixed<Store,shift> x)
	{
		static_assert((shift & 1) == 0);
		constexpr auto resultShift = shift/2;
		Fixed<Store,resultShift> result;
		result.raw = ::sqrt(x.raw);
		return result;
	}

	using intp8 = Fixed<int32_t,8>;
	using intp12 = Fixed<int32_t,12>;
	using intp16 = Fixed<int32_t,16>;
	using intp24 = Fixed<int32_t,24>;

	constexpr intp8 operator""_p8(long double x) { return intp8(float(x)); }
	constexpr intp8 operator""_p8(unsigned long long x) { return intp8(long(x)); }
	constexpr intp12 operator""_p12(long double x) { return intp12(float(x)); }
	constexpr intp12 operator""_p12(unsigned long long x) { return intp12(long(x)); }
	constexpr intp16 operator""_p16(long double x) { return intp16(float(x)); }
	constexpr intp16 operator""_p16(unsigned long long x) { return intp16(long(x)); }
	constexpr intp24 operator""_p24(long double x) { return intp24(float(x)); }
	constexpr intp24 operator""_p24(unsigned long long x) { return intp24(long(x)); }

} // namespace math