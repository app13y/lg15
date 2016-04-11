#if defined USE_OPTIMISED_IMPLEMENTATION && !defined __SSE2__

#include <string.h>
#include "gosthopper.h"
#include "tables.h"
#include "long_tables.h"

#define Maximum(left, right) (((left) >= (right)) ? (left) : (right))


static uint64_t *getMutableRoundKey(uint64_t *roundKeys, size_t roundKeyIndex) {
    return roundKeys + roundKeyIndex * BlockLengthInLimbs;
}


static const void *getConstantRoundKey(const uint64_t *roundKeys, size_t roundKeyIndex) {
    return roundKeys + roundKeyIndex * BlockLengthInLimbs;
}


static void *delayedShiftMemoryPointer(uint64_t **memory, size_t shiftInLimbs) {
    uint64_t *original = *memory;
    *memory += shiftInLimbs;
    return original;
}


static void applySTransformation(
        unsigned char *block
) {
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block[byteIndex_] = Pi[block[byteIndex_]];
    }
}


static void applyInversedSTransformation(
        unsigned char *block
) {
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block[byteIndex_] = InversedPi[block[byteIndex_]];
    }
}


static void applyXTransformation(
        const uint64_t *key,
        const uint64_t *input,
        uint64_t *output
) {
    output[0] = input[0] ^ key[0];
    output[1] = input[1] ^ key[1];
}


static void swapBlocks(
        uint64_t *left,
        uint64_t *right
) {
    /* Inequality left != right shall hold. */
    left[0] = left[0] ^ right[0];
    left[1] = left[1] ^ right[1];

    right[0] = left[0] ^ right[0];
    right[1] = left[1] ^ right[1];

    left[0] = left[0] ^ right[0];
    left[1] = left[1] ^ right[1];
}


static void applyLSTransformation(
        const uint64_t *input,
        uint64_t *output
) {
    size_t index_;

    memset(output, 0, BlockLengthInBytes);
    for (index_ = 0; index_ < 8; ++index_) {
        uint64_t low_, high_;
        memcpy(&low_, precomputedLSTable[0 + index_][input[0] >> 0x08 * index_ & 0xff], sizeof low_);
        memcpy(&high_, precomputedLSTable[0 + index_][input[0] >> 0x08 * index_ & 0xff] + sizeof low_, sizeof high_);
        output[0] ^= low_;
        output[1] ^= high_;
    }
    for (index_ = 0; index_ < 8; ++index_) {
        uint64_t low_, high_;
        memcpy(&low_, precomputedLSTable[8 + index_][input[1] >> 0x08 * index_ & 0xff], sizeof low_);
        memcpy(&high_, precomputedLSTable[8 + index_][input[1] >> 0x08 * index_ & 0xff] + sizeof low_, sizeof high_);
        output[0] ^= low_;
        output[1] ^= high_;
    }
}


static void applyInversedLSTransformation(
        const uint64_t *input,
        uint64_t *output
) {
    size_t index_;

    memset(output, 0, BlockLengthInBytes);

    for (index_ = 0; index_ < 8; ++index_) {
        uint64_t low_, high_;
        memcpy(&low_, precomputedInversedLSTable[0 + index_][input[0] >> 0x08 * index_ & 0xff], sizeof low_);
        memcpy(&high_, precomputedInversedLSTable[0 + index_][input[0] >> 0x08 * index_ & 0xff] + sizeof low_, sizeof high_);
        output[0] ^= low_;
        output[1] ^= high_;
    }
    for (index_ = 0; index_ < 8; ++index_) {
        uint64_t low_, high_;
        memcpy(&low_, precomputedInversedLSTable[8 + index_][input[1] >> 0x08 * index_ & 0xff], sizeof low_);
        memcpy(&high_, precomputedInversedLSTable[8 + index_][input[1] >> 0x08 * index_ & 0xff] + sizeof low_, sizeof high_);
        output[0] ^= low_;
        output[1] ^= high_;
    }
}


#define WorkspaceOfApplyFTransformation_ (BlockLengthInBytes * 2)
static void applyFTransformation(
        const uint8_t *roundConstant,
        uint64_t *left,
        uint64_t *right,
        uint64_t *memory
) {
    uint64_t *leftBranched_ = delayedShiftMemoryPointer(&memory, BlockLengthInLimbs);
    uint64_t *cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInLimbs);

    /* Hoping compiler would optimise key_ variable and memory copying out. */
    uint64_t key_[BlockLengthInLimbs];
    memcpy(key_, roundConstant, BlockLengthInBytes);

    applyXTransformation(key_, left, cache_);
    applyLSTransformation(cache_, leftBranched_);
    applyXTransformation(leftBranched_, right, right);
    swapBlocks(left, right);
}


#define WorkspaceOfScheduleEncryptionRoundKeys_ WorkspaceOfApplyFTransformation_
const size_t WorkspaceOfScheduleEncryptionRoundKeys = WorkspaceOfScheduleEncryptionRoundKeys_;


void scheduleEncryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    size_t roundKeyPairIndex_ = 0, constantIndex_ = 0;
    uint64_t *memory_ = memory;
    const uint8_t *reinterpretedKey_ = key;

    memcpy(getMutableRoundKey(roundKeys, 0), reinterpretedKey_, BlockLengthInBytes);
    memcpy(getMutableRoundKey(roundKeys, 1), reinterpretedKey_ + BlockLengthInBytes, BlockLengthInBytes);

    for (; roundKeyPairIndex_ < NumberOfRounds / 2 - 1; ++roundKeyPairIndex_) {
        size_t feistelRoundIndex_ = 0;
        memcpy(getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
               getMutableRoundKey(roundKeys, (roundKeyPairIndex_ - 1) * 2 + 2),
               BlockLengthInBytes * 2);

        for (; feistelRoundIndex_ < NumberOfRoundsInKeySchedule; ++feistelRoundIndex_) {
            applyFTransformation(roundConstants[constantIndex_++],
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 3),
                                 memory_);
        }
    }
}


#define WorkspaceOfScheduleDecryptionRoundKeys_ (BlockLengthInBytes + WorkspaceOfScheduleEncryptionRoundKeys_)
const size_t WorkspaceOfScheduleDecryptionRoundKeys = WorkspaceOfScheduleDecryptionRoundKeys_;
void scheduleDecryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    uint64_t *cache_ = NULL, *memory_ = memory;
    size_t round_ = 1;

    cache_ = delayedShiftMemoryPointer(&memory_, BlockLengthInLimbs);
    scheduleEncryptionRoundKeys(roundKeys, key, memory_);

    for (; round_ <= NumberOfRoundsInKeySchedule; ++round_) {
        memcpy(cache_, getConstantRoundKey(roundKeys, round_), BlockLengthInBytes);
        applySTransformation((unsigned char*)cache_);
        applyInversedLSTransformation(cache_, getMutableRoundKey(roundKeys, round_));
    }
}


const size_t WorkspaceOfEncryptBlock = BlockLengthInBytes;
void encryptBlock(
        const void *roundKeys,
        void *data,
        void *memory
) {
    uint64_t *cache_ = memory;
    size_t round_ = 0;

    while (round_ < NumberOfRounds - 1) {
        applyXTransformation(getConstantRoundKey(roundKeys, round_++), data, cache_);
        applyLSTransformation(cache_, data);
    }
    applyXTransformation(getConstantRoundKey(roundKeys, round_), data, data);
}


const size_t WorkspaceOfDecryptBlock = BlockLengthInBytes;
void decryptBlock(
        const void *roundKeys,
        void *data,
        void *memory
) {
    uint64_t *cache_ = memory;
    size_t round_ = NumberOfRounds - 1;

    applyXTransformation(getConstantRoundKey(roundKeys, round_--), data, data);

    applySTransformation(data);
    applyInversedLSTransformation(data, cache_);
    applyInversedLSTransformation(cache_, data);
    applyXTransformation(getConstantRoundKey(roundKeys, round_--), data, cache_);

    while (round_ > 0) {
        applyInversedLSTransformation(cache_, data);
        applyXTransformation(getConstantRoundKey(roundKeys, round_--), data, cache_);
    }

    applyInversedSTransformation((unsigned char *)cache_);
    applyXTransformation(getConstantRoundKey(roundKeys, round_), cache_, data);
}


#else
typedef void ISOCompilerHappiness_t;
#endif
