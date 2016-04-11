#if defined USE_OPTIMISED_IMPLEMENTATION && !defined __SSE2__

#include <string.h>
#include "gosthopper.h"
#include "tables.h"
#include "long_tables.h"

#define Maximum(left, right) (((left) >= (right)) ? (left) : (right))

union longTable_t {
    const uint8_t (*asBytes)[256][16];
    const uint64_t (*asQWords)[256][2];
};

union constantMemoryLocation_t {
    const uint8_t *asBytes;
    const uint64_t *asQWords;
};

union mutableMemoryLocation_t {
    uint8_t *asBytes;
    uint64_t *asQWords;
};


static uint64_t *getMutableRoundKey(void *roundKeys, size_t roundKeyIndex) {
    union mutableMemoryLocation_t roundKey_;
    uint8_t *reinterpretedRoundKeys_ = roundKeys;
    roundKey_.asBytes = &reinterpretedRoundKeys_[roundKeyIndex * BlockLengthInBytes];
    return roundKey_.asQWords;
}


static const uint64_t *getConstantRoundKey(const void *roundKeys, size_t roundKeyIndex) {
    union constantMemoryLocation_t roundKey_;
    const uint8_t *reinterpretedRoundKeys_ = roundKeys;
    roundKey_.asBytes = &reinterpretedRoundKeys_[roundKeyIndex * BlockLengthInBytes];
    return roundKey_.asQWords;
}


static void *delayedShiftMemoryPointer(void **memory, size_t shiftInBytes) {
    void *original = *memory;
    *memory = (uint8_t *) (*memory) + shiftInBytes;
    return original;
}


static void applySTransformation(
        uint64_t *block
) {
    union mutableMemoryLocation_t block_;
    size_t byteIndex_ = 0;

    block_.asQWords = block;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block_.asBytes[byteIndex_] = Pi[block_.asBytes[byteIndex_]];
    }
}


static void applyInversedSTransformation(
        uint64_t *block
) {
    union mutableMemoryLocation_t block_;
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


#define WorkspaceOfApplyFTransformation_ (BlockLengthInBytes * 2)
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


#define WorkspaceOfFetchKeyScheduleRoundConstant_ BlockLengthInBytes
static void fetchKeyScheduleRoundConstant(
        uint8_t index,
        uint64_t *roundConstant,
        void *memory
) {
    union mutableMemoryLocation_t temporary_;

    temporary_.asBytes = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    memset(temporary_.asBytes, 0, BlockLengthInBytes);

    /* WARNING: endianness-specific code here! */
    temporary_.asBytes[BlockLengthInBytes - 1] = index;

    applyInversedSTransformation(temporary_.asQWords);
    applyLSTransformation(temporary_.asQWords, roundConstant);
}


#define WorkspaceOfScheduleEncryptionRoundKeys_ (BlockLengthInBytes +                               \
                                                 Maximum(WorkspaceOfApplyFTransformation_,          \
                                                         WorkspaceOfFetchKeyScheduleRoundConstant_))
const size_t WorkspaceOfScheduleEncryptionRoundKeys = WorkspaceOfScheduleEncryptionRoundKeys_;
void scheduleEncryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    uint64_t *feistelRoundKey_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    size_t roundKeyPairIndex_ = 0;
    uint8_t index_ = 0x01;
    const uint8_t *reinterpretedKey_ = key;

    memcpy(getMutableRoundKey(roundKeys, 0), &reinterpretedKey_[0], BlockLengthInBytes);
    memcpy(getMutableRoundKey(roundKeys, 1), &reinterpretedKey_[BlockLengthInBytes], BlockLengthInBytes);

    for (; roundKeyPairIndex_ < NumberOfRounds / 2 - 1; ++roundKeyPairIndex_) {
        size_t feistelRoundIndex_ = 0;
        memcpy(getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
               getMutableRoundKey(roundKeys, (roundKeyPairIndex_ - 1) * 2 + 2),
               BlockLengthInBytes * 2);

        for (;
                feistelRoundIndex_ < NumberOfRoundsInKeySchedule;
                ++feistelRoundIndex_) {
            fetchKeyScheduleRoundConstant(index_++, feistelRoundKey_, memory);
            applyFTransformation(feistelRoundKey_,
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 3),
                                 memory);
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
    uint64_t *cache_;
    size_t round_ = 1;

    cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    scheduleEncryptionRoundKeys(roundKeys, key, memory);

    for (; round_ <= 8; ++round_) {
        memcpy(cache_, getConstantRoundKey(roundKeys, round_), BlockLengthInBytes);
        applySTransformation(cache_);
        applyInversedLSTransformation(cache_, getMutableRoundKey(roundKeys, round_));
    }
}


const size_t WorkspaceOfEncryptBlock = BlockLengthInBytes;
void encryptBlock(
        const void *roundKeys,
        void *data,
        void *memory
) {
    uint64_t *cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    size_t round_ = 0;
    for (; round_ < NumberOfRounds - 1; ++round_) {
        applyXTransformation(getConstantRoundKey(roundKeys, round_), data, cache_);
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
    size_t round_ = NumberOfRounds - 1;
    uint64_t *cache_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    applyXTransformation(getConstantRoundKey(roundKeys, round_--), data, data);

    applySTransformation(data);
    applyInversedLSTransformation(data, cache_);
    applyInversedLSTransformation(cache_, data);
    applyXTransformation(getConstantRoundKey(roundKeys, round_--), data, cache_);

    for (; round_ > 0; round_--) {
        applyInversedLSTransformation(cache_, data);
        applyXTransformation(getConstantRoundKey(roundKeys, round_), data, cache_);
    }

    applyInversedSTransformation(cache_);
    applyXTransformation(getConstantRoundKey(roundKeys, round_), cache_, data);
}


#else
typedef void ISOCompilerHappiness_t;
#endif
