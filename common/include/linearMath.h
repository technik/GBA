#pragma once

#include <cstddef>
#include <cstdint>

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

		constexpr explicit FixedPoint(float x)
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
            result.raw = rounded>>diff;
            return result;
		}

		constexpr int32_t floor() const { return raw>>shift; }
		constexpr int32_t round() const {
			static_assert(shift>0);
			constexpr int32_t half = 1<<(shift-1);
			return (raw+half)>>shift;
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
	constexpr FixedPoint<shift> operator*(
		FixedPoint<shift> a,
		FixedPoint<shift> b)
	{
		static_assert(shift>1);

		FixedPoint<shift> result{};

		int32_t sumRaw = a.raw * b.raw;
		sumRaw += (1<<(shift-1)); // Unbiased round by adding +0.5
		
		result.raw = sumRaw >> shift;
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