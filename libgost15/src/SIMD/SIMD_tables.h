#if !defined LIBGOST15_SIMD_TABLES_HEADER_INCLUDED_
#define LIBGOST15_SIMD_TABLES_HEADER_INCLUDED_

#include <libgost15/platform.h>

/* Crude alignment macros. */
#if defined _MSC_VER
#define ALIGNED(what) __declspec(align(16)) what
#elif defined(__GNUC__)
#define ALIGNED(what) what __attribute__((aligned(16)))
#else
#define ALIGNED(what) what
#endif

extern const uint8_t ALIGNED(bitmask[16]);
extern const uint8_t ALIGNED(precomputedInversedLSTable[16 * 256 * 16]);
extern const uint8_t ALIGNED(precomputedLSTable[16 * 256 * 16]);

#endif
