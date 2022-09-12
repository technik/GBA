#pragma once

#ifdef _WIN32
#define FORCE_INLINE
#elif defined(GBA)
#define FORCE_INLINE __attribute__((always_inline))
#endif