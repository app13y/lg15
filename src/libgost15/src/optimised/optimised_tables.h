#if !defined LIBGOST15_OPTIMISED_TABLES_HEADER_INCLUDED_
#define LIBGOST15_OPTIMISED_TABLES_HEADER_INCLUDED_

#include <libgost15/platform.h>

union qword_t {
    uint8_t asBytes[8];
    uint64_t asQWord;
};

extern const union qword_t roundConstantsLeft[32];
extern const union qword_t roundConstantsRight[32];
extern const union qword_t precomputedLSTableLeft[16][256];
extern const union qword_t precomputedLSTableRight[16][256];
extern const union qword_t precomputedInversedLSTableLeft[16][256];
extern const union qword_t precomputedInversedLSTableRight[16][256];

#endif
