#if !defined USE_OPTIMISED_IMPLEMENTATION

#include <string.h>
#include "gosthopper.h"
#include "tables.h"


static void *delayedShiftMemoryPointer(void **memory, size_t shiftInBytes) {
    void *original = *memory;
    *memory = (uint8_t *) (*memory) + shiftInBytes;
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
        const uint8_t *key,
        const uint8_t *input,
        uint8_t *output
) {
    const uint64_t *reinterpretedKey_ = (const uint64_t *) key;
    const uint64_t *reinterpretedInput_ = (const uint64_t *) input;
    uint64_t *reinterpretedOutput_ = (uint64_t *) output;

    reinterpretedOutput_[0] = reinterpretedKey_[0] ^ reinterpretedInput_[0];
    reinterpretedOutput_[1] = reinterpretedKey_[1] ^ reinterpretedInput_[1];
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


static void applyFTransformation(
        const uint8_t *key,
        uint8_t *left,
        uint8_t *right,
        void *memory
) {
    uint8_t *left_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    memcpy(left_, left, BlockLengthInBytes);
    applyXSLTransformation(key, left_, temporary_);
    applyXTransformation(left_, right, right);
    swapBlocks(left, right);
}


static void fetchKeyScheduleRoundConstant(
        uint8_t index,
        uint8_t *roundConstant,
        void *memory
) {
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    memset(temporary_, 0, BlockLengthInBytes);

    temporary_[BlockLengthInBytes - 1] = index;
    applyLTransformation(temporary_, roundConstant);
}


static void scheduleRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    uint8_t *feistelRoundKey_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    uint8_t index_ = 0x01;
    uint8_t roundKeyPairIndex_ = 0;

    uint8_t *reinterpretedRoundKeys_ = roundKeys;
    const uint8_t *key_ = key;

    memcpy(&reinterpretedRoundKeys_[0], key_, BlockLengthInBytes);
    memcpy(&reinterpretedRoundKeys_[BlockLengthInBytes], key_ + BlockLengthInBytes, BlockLengthInBytes);

    for (; roundKeyPairIndex_ < 4; ++roundKeyPairIndex_) {
        size_t feistelRoundIndex_ = 0;

        memcpy(&reinterpretedRoundKeys_[(2 + 2 * roundKeyPairIndex_) * BlockLengthInBytes],
               &reinterpretedRoundKeys_[(2 + 2 * (roundKeyPairIndex_ - 1)) * BlockLengthInBytes],
               BlockLengthInBytes * 2);

        for (; feistelRoundIndex_ < NumberOfRoundsInKeySchedule; ++feistelRoundIndex_) {
            fetchKeyScheduleRoundConstant(index_++, feistelRoundKey_, memory);
            applyFTransformation(feistelRoundKey_,
                                 &reinterpretedRoundKeys_[(2 + 2 * roundKeyPairIndex_) * BlockLengthInBytes],
                                 &reinterpretedRoundKeys_[(3 + 2 * roundKeyPairIndex_) * BlockLengthInBytes],
                                 memory);
        }
    }
}


void scheduleEncryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    scheduleRoundKeys(roundKeys, key, memory);
}


void scheduleDecryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
) {
    scheduleRoundKeys(roundKeys, key, memory);
}


void encryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
) {
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    const uint8_t *reinterpretedRoundKeys_ = roundKeys;
    size_t roundIndex_ = 0;
    for (; roundIndex_ < NumberOfRounds - 1; ++roundIndex_) {
        applyXSLTransformation(&reinterpretedRoundKeys_[roundIndex_ * BlockLengthInBytes],
                               block,
                               temporary_);
    }
    applyXTransformation(&reinterpretedRoundKeys_[roundIndex_ * BlockLengthInBytes], block, block);
}


void decryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
) {
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    const uint8_t *reinterpretedRoundKeys_ = roundKeys;
    size_t roundIndex_ = 0;
    for (roundIndex_ = NumberOfRounds - 1; roundIndex_ > 0; --roundIndex_) {
        applyInversedSLXTransformation(&reinterpretedRoundKeys_[roundIndex_ * BlockLengthInBytes],
                                       block,
                                       temporary_);
    }

    applyXTransformation(&reinterpretedRoundKeys_[roundIndex_ * BlockLengthInBytes], block, block);
}


#else
typedef void ISOCompilerHappiness_t;
#endif
