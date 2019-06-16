#include <libgost15/internals/data.h>
#include <libgost15/internals/inline.h>
#include <libgost15/internals/alignas.h>
#include <libgost15/internals/restrict.h>
#include <libgost15/libgost15.h>
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

static inline void
substituteBytes(
    uint8_t * restrict block,
    uint8_t const * restrict sbox
) {
    assert(block);
    assert(sbox);

    for (size_t i = 0; i < BlockLengthInBytes; ++i)
        block[i] = sbox[block[i]];
}

static inline void
directSubstituteBytes(
    uint8_t *block
) {
    substituteBytes(block, lg15_directPi);
}

static inline void
inverseSubstituteBytes(
    uint8_t *block
) {
    substituteBytes(block, lg15_inversePi);
}

static inline uint8_t *
getAt(
    uint8_t const *table,
    unsigned index
) {
    assert(table);
    assert(index < 0x1000);

    return (uint8_t *) &table[index * BlockLengthInBytes];
}

static inline void
xorBlocks(
    void * restrict dst,
    void const * restrict src
) {
    assert(dst);
    assert(isAligned(dst, 8));
    assert(src);
    assert(isAligned(src, 8));

    uint64_t
        *dst_ = dst;
    uint64_t const
        *src_ = src;
    size_t const
        size_ = BlockLengthInBytes / sizeof (uint64_t);

    for (size_t i = 0; i < size_; ++i)
        dst_[i] ^= src_[i];

    return;
}

static inline void
copyBlock(
    void * restrict dst,
    void const * restrict src
) {
    assert(dst);
    assert(isAligned(dst, 8));
    assert(src);
    assert(isAligned(src, 8));

    uint64_t
        *dst_ = dst;
    uint64_t const
        *src_ = src;
    size_t const
        size_ = BlockLengthInBytes / sizeof (uint64_t);

    for (size_t i = 0; i < size_; ++i)
        dst_[i] = src_[i];

    return;
}

static inline void
transform(
    uint8_t * restrict block,
    uint8_t const * restrict table
) {
    assert(block);
    assert(isAligned(block, 8));
    assert(table);

    alignas(8) uint8_t
        buffer[BlockLengthInBytes] = {0};

    xorBlocks(buffer, getAt(table, block[0x0] + 0x000));
    xorBlocks(buffer, getAt(table, block[0x1] + 0x100));
    xorBlocks(buffer, getAt(table, block[0x2] + 0x200));
    xorBlocks(buffer, getAt(table, block[0x3] + 0x300));
    xorBlocks(buffer, getAt(table, block[0x4] + 0x400));
    xorBlocks(buffer, getAt(table, block[0x5] + 0x500));
    xorBlocks(buffer, getAt(table, block[0x6] + 0x600));
    xorBlocks(buffer, getAt(table, block[0x7] + 0x700));
    xorBlocks(buffer, getAt(table, block[0x8] + 0x800));
    xorBlocks(buffer, getAt(table, block[0x9] + 0x900));
    xorBlocks(buffer, getAt(table, block[0xa] + 0xa00));
    xorBlocks(buffer, getAt(table, block[0xb] + 0xb00));
    xorBlocks(buffer, getAt(table, block[0xc] + 0xc00));
    xorBlocks(buffer, getAt(table, block[0xd] + 0xd00));
    xorBlocks(buffer, getAt(table, block[0xe] + 0xe00));
    xorBlocks(buffer, getAt(table, block[0xf] + 0xf00));

    copyBlock(block, buffer);
}

static inline void
directTransform(
    uint8_t *block
) {
    transform(block, lg15_directTable);
}

static inline void
inverseTransform(
    uint8_t *block
) {
    transform(block, lg15_inverseTable);
}

extern void
lg15_encryptBlocks(
    uint8_t const * restrict roundKeys,
    uint8_t * restrict blocks,
    size_t numberOfBlocks
) {
    assert(roundKeys);
    assert(isAligned(roundKeys, 8));
    assert(blocks);
    assert(isAligned(blocks, 8));

    for (size_t i = 0; i < numberOfBlocks; ++i) {
        uint8_t * restrict
            block = getAt(blocks, i);
        unsigned
            round = 0;

        xorBlocks(block, getAt(roundKeys, round));
        ++round;

        for (; round < NumberOfRounds; ++round) {
            directTransform(blocks);
            xorBlocks(blocks, getAt(roundKeys, round));
        }
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
        uint8_t * restrict
            block = getAt(blocks, i);
        unsigned
            round = NumberOfRounds - 1;

        xorBlocks(block, getAt(roundKeys, round));
        --round;

        directSubstituteBytes(block);
        inverseTransform(block);
        inverseTransform(block);
        xorBlocks(block, getAt(roundKeys, round));
        --round;

        for (; round > 0; --round) {
            inverseTransform(block);
            xorBlocks(block, getAt(roundKeys, round));
        }

        inverseSubstituteBytes(block);
        xorBlocks(block, getAt(roundKeys, round));
    }

    return;
}


/*
 * Scheduling round keys
 */

static inline uint8_t *
constantAt(
    unsigned index
) {
    return getAt(lg15_constants, index);
}

static inline void
feistelRoundWithoutSwap(
    uint8_t *left,
    uint8_t const *right,
    unsigned constantIndex
) {
    alignas(8) uint8_t
        buffer[BlockLengthInBytes];

    copyBlock(buffer, right);
    xorBlocks(buffer, constantAt(constantIndex));
    directTransform(buffer);
    xorBlocks(left, buffer);
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

    unsigned
        i = 0;

    copyBlock(getAt(roundKeys, 0), getAt(key, 0));
    copyBlock(getAt(roundKeys, 1), getAt(key, 1));
    i += 2;

    for (unsigned constantIndex = 0; i != NumberOfRounds; i += 2) {
        uint8_t
            *left = getAt(roundKeys, i),
            *right = getAt(roundKeys, i + 1);

        copyBlock(left, getAt(roundKeys, i - 2));
        copyBlock(right, getAt(roundKeys, i - 1));

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
    lg15_scheduleEncryptionRoundKeys(roundKeys, key);

    for (unsigned i = 1; i <= NumberOfRounds - 2; ++i) {
        directSubstituteBytes(getAt(roundKeys, i));
        inverseTransform(getAt(roundKeys, i));
    }
}
