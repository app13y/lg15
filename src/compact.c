#if !defined USE_OPTIMISED_IMPLEMENTATION

#include <string.h>
#include "gosthopper.h"
#include "tables.h"

#define Maximum(left, right) (((left) >= (right)) ? (left) : (right))

static uint8_t *getMutableRoundKey(uint8_t *roundKeys, size_t roundKeyIndex) {
    return roundKeys + roundKeyIndex * BlockLengthInBytes;
}


static const uint8_t *getConstantRoundKey(const uint8_t *roundKeys, size_t roundKeyIndex) {
    return roundKeys + roundKeyIndex * BlockLengthInBytes;
}


static void *delayedShiftMemoryPointer(uint8_t **memory, size_t shiftInBytes) {
    uint8_t *original = *memory;
    *memory += shiftInBytes;
    return original;
}


static uint8_t multiplyInGF256(
        uint8_t left,
        uint8_t right
) {
    return !left || !right ?
           (uint8_t) 0 :
           exponentialTable[(logarithmicTable[left] + logarithmicTable[right]) % 0xff];
}


static void applyXTransformation(
        const uint8_t * __restrict key,
        const uint8_t *input,
        uint8_t * __restrict output
) {
    /* Hoping compiler would merge consecutive byte xor's into couple of full register xors. */
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; byteIndex_ += 8) {
        output[byteIndex_ + 0] = input[byteIndex_ + 0] ^ key[byteIndex_ + 0];
        output[byteIndex_ + 1] = input[byteIndex_ + 1] ^ key[byteIndex_ + 1];
        output[byteIndex_ + 2] = input[byteIndex_ + 2] ^ key[byteIndex_ + 2];
        output[byteIndex_ + 3] = input[byteIndex_ + 3] ^ key[byteIndex_ + 3];

        output[byteIndex_ + 4] = input[byteIndex_ + 4] ^ key[byteIndex_ + 4];
        output[byteIndex_ + 5] = input[byteIndex_ + 5] ^ key[byteIndex_ + 5];
        output[byteIndex_ + 6] = input[byteIndex_ + 6] ^ key[byteIndex_ + 6];
        output[byteIndex_ + 7] = input[byteIndex_ + 7] ^ key[byteIndex_ + 7];
    }
}


static void applySTransformation(
        uint8_t *block
) {
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block[byteIndex_] = Pi[block[byteIndex_]];
    }
}


static void applyInversedSTransformation(
        uint8_t *block
) {
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block[byteIndex_] = InversedPi[block[byteIndex_]];
    }
}


static void applyLTransformation(
        const uint8_t *input,
        uint8_t *output
) {
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        size_t addendIndex_ = 0;
        for (output[byteIndex_] = 0; addendIndex_ < BlockLengthInBytes; ++addendIndex_) {
            output[byteIndex_] ^= multiplyInGF256(LTransformationMatrix[addendIndex_][byteIndex_],
                                                  input[addendIndex_]);
        }
    }
}


static void applyInversedLTransformation(
        const uint8_t *input,
        uint8_t *output
) {
    size_t byteIndex_ = 0;
    for (; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        size_t addendIndex_ = 0;
        output[byteIndex_] = 0x00;

        for (; addendIndex_ < BlockLengthInBytes; ++addendIndex_) {
            output[byteIndex_] ^= multiplyInGF256(inversedLTransformationMatrix[addendIndex_][byteIndex_],
                                                  input[addendIndex_]);
        }
    }
}


static void applyXSLTransformation(
        const uint8_t *key,
        uint8_t *block,
        uint8_t *temporary
) {
    applyXTransformation(key, block, temporary);
    applySTransformation(temporary);
    applyLTransformation(temporary, block);
}


static void applyInversedSLXTransformation(
        const uint8_t *key,
        uint8_t *block,
        uint8_t *temporary
) {
    applyXTransformation(key, block, temporary);
    applyInversedLTransformation(temporary, block);
    applyInversedSTransformation(block);
}


static void swapBlocks(
        uint8_t *left,
        uint8_t *right
) {
    size_t byteIndex_ = 0;
    for (; left != right && byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        left[byteIndex_] ^= right[byteIndex_];
        right[byteIndex_] ^= left[byteIndex_];
        left[byteIndex_] ^= right[byteIndex_];
    }
}


#define WorkspaceOfApplyFTransformation_ (BlockLengthInBytes * 2)


static void applyFTransformation(
        const uint8_t *key,
        uint8_t *left,
        uint8_t *right,
        uint8_t *memory
) {
    uint8_t *left_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    memcpy(left_, left, BlockLengthInBytes);
    applyXSLTransformation(key, left_, temporary_);
    applyXTransformation(left_, right, right);
    swapBlocks(left, right);
}


#define WorkspaceOfFetchKeyScheduleRoundConstant_ BlockLengthInBytes


static void fetchKeyScheduleRoundConstant(
        uint8_t index,
        uint8_t *roundConstant,
        uint8_t *memory
) {
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    memset(temporary_, 0, BlockLengthInBytes);
    temporary_[BlockLengthInBytes - 1] = index;
    applyLTransformation(temporary_, roundConstant);
}


#define WorkspaceOfScheduleRoundKeys_ (BlockLengthInBytes + \
                                       Maximum(WorkspaceOfFetchKeyScheduleRoundConstant_, \
                                               WorkspaceOfApplyFTransformation_))
static void scheduleRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {

    uint8_t index_ = 0x01;
    size_t roundKeyPairIndex_ = 0;
    const uint8_t *key_ = key;
    uint8_t *feistelRoundKey_ = NULL, *memory_ = memory;

    feistelRoundKey_ = delayedShiftMemoryPointer(&memory_, BlockLengthInBytes);

    memcpy(getMutableRoundKey(roundKeys, 0), key_, BlockLengthInBytes);
    memcpy(getMutableRoundKey(roundKeys, 1), key_ + BlockLengthInBytes, BlockLengthInBytes);

    for (; roundKeyPairIndex_ < 4; ++roundKeyPairIndex_) {
        size_t feistelRoundIndex_ = 0;

        memcpy(getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
               getMutableRoundKey(roundKeys, (roundKeyPairIndex_ - 1) * 2 + 2),
               BlockLengthInBytes * 2);

        for (; feistelRoundIndex_ < NumberOfRoundsInKeySchedule; ++feistelRoundIndex_) {
            fetchKeyScheduleRoundConstant(index_++, feistelRoundKey_, memory_);
            applyFTransformation(feistelRoundKey_,
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 3),
                                 memory_);
        }
    }
}


const size_t WorkspaceOfScheduleEncryptionRoundKeys = WorkspaceOfScheduleRoundKeys_;
void scheduleEncryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    scheduleRoundKeys(roundKeys, key, memory);
}


const size_t WorkspaceOfScheduleDecryptionRoundKeys = WorkspaceOfScheduleRoundKeys_;
void scheduleDecryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    scheduleRoundKeys(roundKeys, key, memory);
}


const size_t WorkspaceOfEncryptBlock = BlockLengthInBytes;
void encryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
) {
    uint8_t *cache_ = memory;
    size_t round_ = 0;

    while (round_ < NumberOfRounds - 1) {
        applyXSLTransformation(getConstantRoundKey(roundKeys, round_++),
                               block,
                               cache_);
    }
    applyXTransformation(getConstantRoundKey(roundKeys, round_), block, block);
}


const size_t WorkspaceOfDecryptBlock = BlockLengthInBytes;
void decryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
) {
    uint8_t *cache_ = memory;
    size_t round_ = NumberOfRounds - 1;

    while (round_ > 0) {
        applyInversedSLXTransformation(getConstantRoundKey(roundKeys, round_--), block, cache_);
    }
    applyXTransformation(getConstantRoundKey(roundKeys, round_), block, block);
}


#else
typedef void ISOCompilerHappiness_t;
#endif
