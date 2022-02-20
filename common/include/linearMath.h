#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>

namespace math
{
	template<class T>
	constexpr auto max(T a, T b)
	{
		return b < a ? a : b;
	}

	template<class T>
	constexpr auto min(T a, T b)
	{
		return a < b ? a : b;
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
			static_assert(!std::is_signed_v<store_type>, "Can't round signed types with this method");

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

	// Fixed point math
	template<size_t shift>
	struct FixedPoint
	{
		FixedPoint() = default;

		constexpr explicit FixedPoint(long x)
			: raw(x<<shift)
		{}

		constexpr explicit FixedPoint(long unsigned x)
			: raw(x<<shift)
		{}

		constexpr explicit FixedPoint(float x) : raw(0)
		{
			int32_t integer = int32_t(x);
			float diff = x-integer;
			int32_t fraction = int32_t(diff*(1<<shift));
			raw = (integer<<shift) + fraction;
		}
		
		constexpr explicit FixedPoint(int32_t x, int32_t frac)
			: raw(x<<shift | frac)
		{}

		int32_t raw;

		template<size_t shift2>
		constexpr auto cast_down()
		{
			static_assert(shift2 < shift);
			constexpr int diff = shift-shift2;
			int32_t rounded = raw + (1<<(diff-1));

            FixedPoint<shift2> result;
            result.raw = rounded/(1<<diff); // Division rather than shift, to preserve the negative sign.
            return result;
		}

		constexpr int32_t floor() const { return raw>>shift; }
		constexpr int32_t round() const {
			static_assert(shift>0);
			constexpr int32_t half = 1<<(shift-1);
			return (raw+half)/(1<<shift); // Division rather than shift, to preserve the negative sign.
		}

		void operator+=(FixedPoint<shift> x)
		{
			raw += x.raw;
		}

		void operator-=(FixedPoint<shift> x)
		{
			raw -= x.raw;
		}

		auto& operator=(const FixedPoint<shift>& x)
		{
			raw = x.raw;
			return *this;
		}

		auto& operator=(const FixedPoint<shift>& x) volatile
		{
			raw = x.raw;
			return *this;
		}
	};

	using intp8 = FixedPoint<8>;
	using intp12 = FixedPoint<12>;
	using intp16 = FixedPoint<16>;
	using intp24 = FixedPoint<24>;

	constexpr intp8 operator""_p8(long double x) { return intp8(float(x)); }
	constexpr intp8 operator""_p8(unsigned long long x) { return intp8(long(x)); }
	constexpr intp12 operator""_p12(long double x) { return intp12(float(x)); }
	constexpr intp12 operator""_p12(unsigned long long x) { return intp12(long(x)); }
	constexpr intp16 operator""_p16(long double x) { return intp16(float(x)); }
	constexpr intp16 operator""_p16(unsigned long long x) { return intp16(long(x)); }
	constexpr intp24 operator""_p24(long double x) { return intp24(float(x)); }
	constexpr intp24 operator""_p24(unsigned long long x) { return intp24(long(x)); }

	template<size_t shift>
	constexpr FixedPoint<shift> operator+(
		FixedPoint<shift> a,
		FixedPoint<shift> b)
	{
		FixedPoint<shift> result{};
		result.raw = a.raw + b.raw;
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator+(
		int32_t a,
		FixedPoint<shift> b)
	{
		FixedPoint<shift> result{};
		result.raw = (a<<shift) + b.raw;
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator+(
		FixedPoint<shift> a,
		int32_t b)
	{
		return b+a;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator-(
		FixedPoint<shift> a,
		FixedPoint<shift> b)
	{
		FixedPoint<shift> result{};
		result.raw = a.raw - b.raw;
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator-(
		FixedPoint<shift> x)
	{
		FixedPoint<shift> result{};
		result.raw = -x.raw;
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator*(
		FixedPoint<shift> a,
		FixedPoint<shift> b)
	{
		static_assert(shift>1);

		FixedPoint<shift> result{};

		int32_t sumRaw = a.raw * b.raw;
		sumRaw += (1<<(shift-1)); // Unbiased round by adding +0.5
		
		result.raw = sumRaw / (1<<shift); // Division rather than shift, to preserve the negative sign.
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator*(
		FixedPoint<shift> a,
		int32_t b)
	{
		FixedPoint<shift> result{};
		result.raw = a.raw * b;
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator*(
		int32_t a,
		FixedPoint<shift> b)
	{
		return b*a;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator/(
		FixedPoint<shift> a,
		FixedPoint<shift> b)
	{
		int32_t extendedNum = a.raw<<shift;
		int32_t ratio = extendedNum / b.raw;

		FixedPoint<shift> result{};
		result.raw = ratio;
		return result;
	}

	template<size_t shift>
	constexpr FixedPoint<shift> operator/(
		FixedPoint<shift> num,
		int32_t den)
	{
		int32_t ratio = num.raw / den;

		FixedPoint<shift> result{};
		result.raw = ratio;
		return result;
	}

	template<size_t shift>
	constexpr bool operator<(
		const FixedPoint<shift>& a,
		const FixedPoint<shift>& b)
	{
		return a.raw < b.raw;
	}

} // namespace math