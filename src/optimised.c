#if defined USE_OPTIMISED_IMPLEMENTATION && !defined USE_SSE_INSTRUCTIONS

#include <string.h>
#include "gosthopper.h"
#include "tables.h"
#include "long_tables.h"


union longTable_t {
    const uint8_t (*asBytes)[256][16];
    const uint64_t (*asQWords)[256][2];
};

union block_t {
    uint64_t *asQWords;
    uint8_t *asBytes;
};


static void *delayedShiftMemoryPointer(void **memory, size_t shiftInBytes) {
    void *original = *memory;
    *memory = (uint8_t *) (*memory) + shiftInBytes;
    return original;
}


static void applySTransformation(
        uint64_t *block
) {
    union block_t block_;
    size_t byteIndex_ = 0;

    block_.asQWords = block;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block_.asBytes[byteIndex_] = Pi[block_.asBytes[byteIndex_]];
    }
}


static void applyInversedSTransformation(
        uint64_t *block
) {
    union block_t block_;
    size_t byteIndex_ = 0;

    block_.asQWords = block;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block_.asBytes[byteIndex_] = InversedPi[block_.asBytes[byteIndex_]];
    }
}


static void swapBlocks(
        uint64_t *left,
        uint64_t *right
) {
    if (left != right) {
        left[0] = left[0] ^ right[0];
        left[1] = left[1] ^ right[1];

        right[0] = left[0] ^ right[0];
        right[1] = left[1] ^ right[1];

        left[0] = left[0] ^ right[0];
        left[1] = left[1] ^ right[1];
    }
}


static void applyLSTransformation(
        const uint64_t *input,
        uint64_t *output
) {
    size_t index_;
    union longTable_t table_ = {precomputedLSTable};

    memset(output, 0, BlockLengthInBytes);

    for (index_ = 0; index_ < 8; ++index_) {
        output[0] ^= table_.asQWords[0 + index_][input[0] >> 0x08 * index_ & 0xff][0];
        output[1] ^= table_.asQWords[0 + index_][input[0] >> 0x08 * index_ & 0xff][1];
    }

    for (index_ = 0; index_ < 8; ++index_) {
        output[0] ^= table_.asQWords[8 + index_][input[1] >> 0x08 * index_ & 0xff][0];
        output[1] ^= table_.asQWords[8 + index_][input[1] >> 0x08 * index_ & 0xff][1];
    }
}


static void applyInversedLSTransformation(
        const uint64_t *input,
        uint64_t *output
) {
    size_t index_;
    union longTable_t table_ = {precomputedInversedLSTable};

    memset(output, 0, BlockLengthInBytes);

    for (index_ = 0; index_ < 8; ++index_) {
        output[0] ^= table_.asQWords[0 + index_][input[0] >> 0x08 * index_ & 0xff][0];
        output[1] ^= table_.asQWords[0 + index_][input[0] >> 0x08 * index_ & 0xff][1];
    }

    for (index_ = 0; index_ < 8; ++index_) {
        output[0] ^= table_.asQWords[8 + index_][input[1] >> 0x08 * index_ & 0xff][0];
        output[1] ^= table_.asQWords[8 + index_][input[1] >> 0x08 * index_ & 0xff][1];
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


static void applyFTransformation(
        const uint64_t *key,
        uint64_t *left,
        uint64_t *right,
        void *memory
) {
    uint64_t *leftBranched_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    uint64_t *cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    applyXTransformation(key, left, cache_);
    applyLSTransformation(cache_, leftBranched_);
    applyXTransformation(leftBranched_, right, right);
    swapBlocks(left, right);
}


static void fetchKeyScheduleRoundConstant(
        uint8_t index,
        uint64_t *roundConstant,
        void *memory
) {
    union block_t temporary_;

    temporary_.asBytes = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    memset(temporary_.asBytes, 0, BlockLengthInBytes);

    /* WARNING: endianness-specific code here! */
    temporary_.asBytes[BlockLengthInBytes - 1] = index;

    applyInversedSTransformation(temporary_.asQWords);
    applyLSTransformation(temporary_.asQWords, roundConstant);
}


void scheduleEncryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    uint64_t *feistelRoundKey_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    size_t roundKeyPairIndex_ = 0;
    uint8_t index_ = 0x01;
    uint64_t *reinterpretedRoundKeys_ = roundKeys;
    const uint8_t *reinterpretedKey_ = key;

    memcpy(&reinterpretedRoundKeys_[0], &reinterpretedKey_[0], BlockLengthInBytes);
    memcpy(&reinterpretedRoundKeys_[BlockLengthInLimbs], &reinterpretedKey_[BlockLengthInBytes], BlockLengthInBytes);

    for (; roundKeyPairIndex_ < NumberOfRounds / 2 - 1; ++roundKeyPairIndex_) {
        size_t feistelRoundIndex_ = 0;
        memcpy(&reinterpretedRoundKeys_[BlockLengthInLimbs * (roundKeyPairIndex_ * 2 + 2)],
               &reinterpretedRoundKeys_[BlockLengthInLimbs * ((roundKeyPairIndex_ - 1) * 2 + 2)],
               BlockLengthInBytes * 2);

        for (;
                feistelRoundIndex_ < NumberOfRoundsInKeySchedule;
                ++feistelRoundIndex_) {
            fetchKeyScheduleRoundConstant(index_++, feistelRoundKey_, memory);
            applyFTransformation(feistelRoundKey_,
                                 &reinterpretedRoundKeys_[BlockLengthInLimbs * (roundKeyPairIndex_ * 2 + 2)],
                                 &reinterpretedRoundKeys_[BlockLengthInLimbs * (roundKeyPairIndex_ * 2 + 3)],
                                 memory);
        }
    }
}


void scheduleDecryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    uint64_t *cache_;
    uint64_t *reinterpretedRoundKeys_ = roundKeys;
    size_t roundKeyPairIndex_ = 1;

    cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    scheduleEncryptionRoundKeys(roundKeys, key, memory);

    for (; roundKeyPairIndex_ <= 8; ++roundKeyPairIndex_) {
        cache_[0] = reinterpretedRoundKeys_[roundKeyPairIndex_ * BlockLengthInLimbs];
        cache_[1] = reinterpretedRoundKeys_[roundKeyPairIndex_ * BlockLengthInLimbs + 1];

        applySTransformation(cache_);
        applyInversedLSTransformation(cache_,
                                      &reinterpretedRoundKeys_[roundKeyPairIndex_ * BlockLengthInLimbs]);
    }
}


void encryptBlock(
        const void *roundKeys,
        void *data,
        void *memory
) {
    const uint64_t *reinterpretedRoundKeys_ = roundKeys;
    uint64_t *cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    size_t round_ = 0;
    for (; round_ < NumberOfRounds - 1; ++round_) {
        applyXTransformation(&reinterpretedRoundKeys_[round_ * BlockLengthInLimbs], data, cache_);
        applyLSTransformation(cache_, data);
    }
    applyXTransformation(&reinterpretedRoundKeys_[round_ * BlockLengthInLimbs], data, data);
}


void decryptBlock(
        const void *roundKeys,
        void *data,
        void *memory
) {
    const uint64_t *reinterpretedRoundKeys_ = roundKeys;
    size_t round_ = NumberOfRounds - 1;
    uint64_t *cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    applyXTransformation(&reinterpretedRoundKeys_[round_-- * BlockLengthInLimbs], data, data);

    applySTransformation(data);
    applyInversedLSTransformation(data, cache_);
    applyInversedLSTransformation(cache_, data);
    applyXTransformation(&reinterpretedRoundKeys_[round_-- * BlockLengthInLimbs], data, cache_);

    for (; round_ > 0; round_--) {
        applyInversedLSTransformation(cache_, data);
        applyXTransformation(&reinterpretedRoundKeys_[round_ * BlockLengthInLimbs], data, cache_);
    }

    applyInversedSTransformation(cache_);
    applyXTransformation(&reinterpretedRoundKeys_[round_ * BlockLengthInLimbs], cache_, data);
}


#else
typedef void ISOCompilerHappiness_t;
#endif
