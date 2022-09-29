#pragma once

#ifdef GBA
#include <tonc.h>
#endif
#ifdef _WIN32
#include <cassert>
#define FORCE_INLINE inline
#elif defined(GBA)
#define FORCE_INLINE __attribute__((always_inline))
#endif

#ifdef _WIN32 // VS workaround for literal suffixes
#define CONSTEVAL constexpr
#else
#define CONSTEVAL consteval
#endif

#ifdef _WIN32
#define IWRAM_CODE
#define EWRAM_CODE
#define IWRAM_DATA
#endif

FORCE_INLINE void dbgAssert(bool x)
{
#ifdef _WIN32
    assert(x);
#endif
}