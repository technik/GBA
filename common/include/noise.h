#pragma once

#include <cstdint>
#include <cstddef>
#include "linearMath.h"

namespace math {

    // Squirrel3 noise implementation from https://www.gdcvault.com/play/1024365/Math-for-Game-Programmers-Noise
    int32_t Squirrel3(int32_t n, int32_t seed = 0)
    {
        // 3 large primes
        constexpr int32_t NOISE1 = 0xb5297a4d; // 0b0110'1000'1110'0011'0001'1101'1010'0100
        constexpr int32_t NOISE2 = 0x68e31da4; // 0b1011'0101'0010'1001'0111'1010'0100'1101
        constexpr int32_t NOISE3 = 0x1b56c4e9; // 0b0001'1011'0101'0110'1100'0100'1110'1001

        n *= NOISE1;
        n += seed;
        n ^= n >> 8;
        n += NOISE2;
        n ^= n << 8;
        n *= NOISE3;
        n ^= n >> 8;

        return n;
    }

    // Maps pseudorandom noise to the [0,1) interval, resulting in a uniform distribution of numbers.
    template<class T>
    T scalarNoise(int32_t n, int32_t seed);

    // Maps pseudorandom noise to the [0,1) interval, resulting in a uniform distribution of numbers.
    template<>
    inline intp8 scalarNoise(int32_t n, int32_t seed)
    {
        intp8 x;
        x.raw = Squirrel3(n, seed) & ((1<<8)-1);
        return x;
    }

    // Maps pseudorandom noise to the [0,1) interval, resulting in a uniform distribution of numbers.
    template<>
    inline intp12 scalarNoise(int32_t n, int32_t seed)
    {
        intp12 x;
        x.raw = Squirrel3(n, seed) & ((1 << 12) - 1);
        return x;
    }

    // Maps pseudorandom noise to the [0,1) interval, resulting in a uniform distribution of numbers.
    template<>
    inline intp16 scalarNoise(int32_t n, int32_t seed)
    {
        intp16 x;
        x.raw = Squirrel3(n, seed) & ((1 << 16) - 1);
        return x;
    }

} // namespace math
