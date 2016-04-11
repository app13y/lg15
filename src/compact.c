#if !defined USE_OPTIMISED_IMPLEMENTATION

#include <string.h>
#include "gosthopper.h"
#include "tables.h"

#define Maximum(left, right) (((left) >= (right)) ? (left) : (right))

union constantMemoryLocation_t {
    const uint8_t *asBytes;
    const uint64_t *asQWords;
};

union mutableMemoryLocation_t {
    uint8_t *asBytes;
    uint64_t *asQWords;
};


static uint8_t *getMutableRoundKey(void *roundKeys, size_t roundKeyIndex) {
    union mutableMemoryLocation_t roundKey_;
    uint8_t *reinterpretedRoundKeys_ = roundKeys;
    roundKey_.asBytes = &reinterpretedRoundKeys_[roundKeyIndex * BlockLengthInBytes];
    return roundKey_.asBytes;
}


static const uint8_t *getConstantRoundKey(const void *roundKeys, size_t roundKeyIndex) {
    union constantMemoryLocation_t roundKey_;
    const uint8_t *reinterpretedRoundKeys_ = roundKeys;
    roundKey_.asBytes = &reinterpretedRoundKeys_[roundKeyIndex * BlockLengthInBytes];
    return roundKey_.asBytes;
}


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
    union constantMemoryLocation_t key_, input_;
    union mutableMemoryLocation_t output_;

    key_.asBytes = key;
    input_.asBytes = input;
    output_.asBytes = output;

    output_.asQWords[0] = key_.asQWords[0] ^ input_.asQWords[0];
    output_.asQWords[1] = key_.asQWords[1] ^ input_.asQWords[1];
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
        void *memory
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
        void *memory
) {
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);
    memset(temporary_, 0, BlockLengthInBytes);

    /* WARNING: endianness-specific code here! */
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
    uint8_t *feistelRoundKey_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    uint8_t index_ = 0x01;
    size_t roundKeyPairIndex_ = 0;
    const uint8_t *key_ = key;

    memcpy(getMutableRoundKey(roundKeys, 0), key_, BlockLengthInBytes);
    memcpy(getMutableRoundKey(roundKeys, 1), key_ + BlockLengthInBytes, BlockLengthInBytes);

    for (; roundKeyPairIndex_ < 4; ++roundKeyPairIndex_) {
        size_t feistelRoundIndex_ = 0;

        memcpy(getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
               getMutableRoundKey(roundKeys, (roundKeyPairIndex_ - 1) * 2 + 2),
               BlockLengthInBytes * 2);

        for (; feistelRoundIndex_ < NumberOfRoundsInKeySchedule; ++feistelRoundIndex_) {
            fetchKeyScheduleRoundConstant(index_++, feistelRoundKey_, memory);
            applyFTransformation(feistelRoundKey_,
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 2),
                                 getMutableRoundKey(roundKeys, roundKeyPairIndex_ * 2 + 3),
                                 memory);
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
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    size_t round_ = 0;
    for (; round_ < NumberOfRounds - 1; ++round_) {
        applyXSLTransformation(getConstantRoundKey(roundKeys, round_),
                               block,
                               temporary_);
    }
    applyXTransformation(getConstantRoundKey(roundKeys, round_), block, block);
}


const size_t WorkspaceOfDecryptBlock = BlockLengthInBytes;


void decryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
) {
    uint8_t *temporary_ = delayedShiftMemoryPointer(&memory, BlockLengthInBytes);

    size_t round_ = 0;
    for (round_ = NumberOfRounds - 1; round_ > 0; --round_) {
        applyInversedSLXTransformation(getConstantRoundKey(roundKeys, round_),
                                       block,
                                       temporary_);
    }
    applyXTransformation(getConstantRoundKey(roundKeys, round_), block, block);
}


#else
typedef void ISOCompilerHappiness_t;
#endif
