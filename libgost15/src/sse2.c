#include <libgost15/internals/data.h>
#include <libgost15/internals/inline.h>
#include <libgost15/internals/alignas.h>
#include <libgost15/internals/restrict.h>
#include <libgost15/libgost15.h>
#include <emmintrin.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

static inline bool
isAligned(
    void const *ptr,
    size_t alignment
) {
    return !(((uintptr_t) ptr) % alignment);
}

/*
 * Processing blocks
 */

static inline __m128i
substituteBytes(
    __m128i chunk,
    uint8_t const *sbox
) {
    assert(sbox);

    alignas(16) uint8_t
        buffer[BlockLengthInBytes];

    _mm_store_si128((__m128i *) buffer, chunk);

    for (size_t i = 0; i < BlockLengthInBytes; ++i)
        buffer[i] = sbox[buffer[i]];

    return _mm_load_si128((__m128i const *) buffer);
}

static inline __m128i
directSubstituteBytes(
     __m128i chunk
) {
    return substituteBytes(chunk, lg15_directPi);
}

static inline __m128i
inverseSubstituteBytes(
    __m128i chunk
) {
    return substituteBytes(chunk, lg15_inversePi);
}

static inline __m128i
getAt(
    uint8_t const *table,
    size_t index
) {
    assert(table); 
    assert(isAligned(table, 16));

    return *((__m128i *) &table[index]);
}

static inline __m128i
getAtIndex(
    uint8_t const *table,
    size_t index
) {
    assert(table); 
    assert(isAligned(table, 16));

    return *((__m128i *) &table[index * BlockLengthInBytes]);
}

static inline void
setAtIndex(
    uint8_t const *table,
    size_t index,
    __m128i value
) {
    assert(table); 
    assert(isAligned(table, 16));

    *((__m128i *) &table[index * BlockLengthInBytes]) = value;
}

static inline __m128i
transform(
    __m128i chunk,
    uint8_t const *table
) {
    __m128i indices;
    indices = _mm_set_epi16(0x0f0e, 0x0d0c, 0x0b0a, 0x0908, 0x0706, 0x0504, 0x0302, 0x0100);

    /* Vectorises lookup displacement evaluation by evaluating
    *   lindices[j] = (input[i] * 16) + (0x1000 * i)
    *                  ~~~~~~~~~~~~~     ~~~~~~~~~~
    *                  shift of          shift of
    *                  i-th byte value   i-th matrix row
    * for bytes i = 0..7 of @chunk.
    */
    __m128i lindices;
    lindices = _mm_unpacklo_epi8(chunk, indices);
    lindices = _mm_slli_epi16(lindices, 4);

    /* Accumulates products from precomputed LUT with pre-evaluated indices
    * for bytes i = 0..7 of @chunk.
    */
    __m128i lcache;
    lcache = getAt(table, _mm_extract_epi16(lindices, 0));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 1)));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 2)));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 3)));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 4)));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 5)));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 6)));
    lcache = _mm_xor_si128(lcache, getAt(table, _mm_extract_epi16(lindices, 7)));

    /* Same as @lindices, but for bytes i = 8..15 of @chunk. */
    __m128i rindices;
    rindices = _mm_unpackhi_epi8(chunk, indices);
    rindices = _mm_slli_epi16(rindices, 4);

    /* Same as @lcache, but for bytes i = 8..15 of @chunk. */
    __m128i rcache;
    rcache = getAt(table, _mm_extract_epi16(rindices, 0));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 1)));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 2)));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 3)));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 4)));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 5)));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 6)));
    rcache = _mm_xor_si128(rcache, getAt(table, _mm_extract_epi16(rindices, 7)));

    return _mm_xor_si128(lcache, rcache);
}

static inline __m128i
directTransform(
    __m128i chunk
) {
    return transform(chunk, lg15_directTable);
}

static inline __m128i
inverseTransform(
    __m128i chunk
) {
    return transform(chunk, lg15_inverseTable);
}

#include <stdio.h>

extern void
lg15_encryptBlocks(
    uint8_t const * restrict roundKeys,
    uint8_t * restrict blocks,
    size_t numberOfBlocks
) {
    assert(roundKeys);
    assert(isAligned(roundKeys, 16));
    assert(blocks);
    assert(isAligned(blocks, 16));

    for (size_t i = 0; i < numberOfBlocks; ++i) {
        __m128i
            block = getAtIndex(blocks, i);
        unsigned
            round = 0;

        block = _mm_xor_si128(block, getAtIndex(roundKeys, round));
        ++round;

        for (; round < NumberOfRounds; ++round) {
            block = directTransform(block);
            block = _mm_xor_si128(block, getAtIndex(roundKeys, round));
        }

        setAtIndex(blocks, i, block);
    }

    return;
}

extern void
lg15_decryptBlocks(
    uint8_t const * restrict roundKeys,
    uint8_t * restrict blocks,
    size_t numberOfBlocks
) {
    assert(roundKeys);
    assert(isAligned(roundKeys, 8));
    assert(blocks);
    assert(isAligned(blocks, 8));

    for (size_t i = 0; i < numberOfBlocks; ++i) {
        __m128i
            block = getAtIndex(blocks, i);
        unsigned
            round = NumberOfRounds - 1;

        block = _mm_xor_si128(block, getAtIndex(roundKeys, round));
        --round;

        block = directSubstituteBytes(block);
        block = inverseTransform(block);
        block = inverseTransform(block);
        block = _mm_xor_si128(block, getAtIndex(roundKeys, round));
        --round;

        for (; round > 0; --round) {
            block = inverseTransform(block);
            block = _mm_xor_si128(block, getAtIndex(roundKeys, round));
        }

        block = inverseSubstituteBytes(block);
        block = _mm_xor_si128(block, getAtIndex(roundKeys, round));

        setAtIndex(blocks, i, block);
    }

    return;
}


/*
 * Scheduling round keys
 */

static inline void
feistelRoundWithoutSwap(
    __m128i *left,
    __m128i const *right,
    size_t constantIndex
) {
    __m128i buffer;

    buffer = _mm_xor_si128(*right, getAtIndex(lg15_constants, constantIndex));
    buffer = directTransform(buffer);
    *left = _mm_xor_si128(*left, buffer);
}

extern void
lg15_scheduleEncryptionRoundKeys(
    uint8_t * restrict roundKeys,
    uint8_t const * restrict key
) {
    assert(roundKeys);
    assert(isAligned(roundKeys, 8));
    assert(key);
    assert(isAligned(key, 8));

     __m128i
          *xmmRoundKeys = (__m128i *) roundKeys;
    unsigned
        i = 0;

    xmmRoundKeys[0] = getAtIndex(key, 0);
    xmmRoundKeys[1] = getAtIndex(key, 1);
    i += 2;

    for (unsigned constantIndex = 0; i != NumberOfRounds; i += 2) {
        __m128i
            *left = &xmmRoundKeys[i],
            *right = &xmmRoundKeys[i + 1];

        *left = xmmRoundKeys[i - 2];
        *right = xmmRoundKeys[i - 1];

        for (size_t round = 0; round < NumberOfRoundsInKeySchedule; round += 2) {
            feistelRoundWithoutSwap(right, left, constantIndex++);
            feistelRoundWithoutSwap(left, right, constantIndex++);
        }
    }

    return;
}

extern void
lg15_scheduleDecryptionRoundKeys(
    uint8_t * restrict roundKeys,
    uint8_t const * restrict key
) {
     __m128i
          *xmmRoundKeys = (__m128i *) roundKeys;

    lg15_scheduleEncryptionRoundKeys(roundKeys, key);

    for (unsigned i = 1; i <= NumberOfRounds - 2; ++i) {
        xmmRoundKeys[i] = directSubstituteBytes(xmmRoundKeys[i]);
        xmmRoundKeys[i] = inverseTransform(xmmRoundKeys[i]);
    }
}
