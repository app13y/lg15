#if defined USE_OPTIMISED_IMPLEMENTATION && !defined __SSE2__

#include <string.h>
#include "gosthopper.h"
#include "tables.h"
#include "optimised_tables.h"


const size_t WorkspaceOfScheduleRoundKeys = BlockLengthInBytes * 2;


static void applySTransformation(
        unsigned char *block
) {
    for (int byteIndex_ = 0; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block[byteIndex_] = Pi[block[byteIndex_]];
    }
}


static void applyInversedSTransformation(
        unsigned char *block
) {
    for (int byteIndex_ = 0; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        block[byteIndex_] = InversedPi[block[byteIndex_]];
    }
}


static void swapBlocks(
        uint64_t *restrict left,
        uint64_t *restrict right
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
        const unsigned char *input,
        uint64_t *output
) {
    uint64_t left_ = 0, right_ = 0;

    for (int index_ = 0; index_ < 16; ++index_) {
        left_ ^= precomputedLSTableLeft[index_][input[index_]].asQWord;
    }

    for (int index_ = 0; index_ < 16; ++index_) {
        right_ ^= precomputedLSTableRight[index_][input[index_]].asQWord;
    }

    output[0] = left_;
    output[1] = right_;
}


static void applyInversedLSTransformation(
        const unsigned char *input,
        uint64_t *output
) {

    uint64_t left_ = 0, right_ = 0;

    for (int index_ = 0; index_ < 16; ++index_) {
        left_ ^= precomputedInversedLSTableLeft[index_][input[index_]].asQWord;
    }

    for (int index_ = 0; index_ < 16; ++index_) {
        right_ ^= precomputedInversedLSTableRight[index_][input[index_]].asQWord;
    }

    output[0] = left_;
    output[1] = right_;
}


static void applyFTransformation(
        int constantIndex,
        uint64_t *restrict left,
        uint64_t *restrict right,
        uint64_t *restrict temp1,
        uint64_t *restrict temp2
) {
    temp1[0] = left[0] ^ roundConstantsLeft[constantIndex].asQWord;
    temp1[1] = left[1] ^ roundConstantsRight[constantIndex].asQWord;

    applyLSTransformation((unsigned char *) temp1, temp2);

    right[0] ^= temp2[0];
    right[1] ^= temp2[1];

    swapBlocks(left, right);
}


void scheduleEncryptionRoundKeys(
        void *restrict roundKeys,
        const void *restrict key,
        void *restrict memory
) {
    uint64_t *memory_ = memory;
    uint64_t *roundKeys_ = roundKeys;

    memcpy(&roundKeys_[0], key, BlockLengthInBytes * 2);

    for (int nextKeyIndex_ = 2, constantIndex_ = 0;
         nextKeyIndex_ != NumberOfRounds;
         nextKeyIndex_ += 2) {
        memcpy(&roundKeys_[2 * (nextKeyIndex_)],
               &roundKeys_[2 * (nextKeyIndex_ - 2)],
               BlockLengthInBytes * 2);

        for (int feistelRoundIndex_ = 0;
             feistelRoundIndex_ < NumberOfRoundsInKeySchedule;
             ++feistelRoundIndex_) {
            applyFTransformation(constantIndex_++,
                                 &roundKeys_[2 * (nextKeyIndex_)],
                                 &roundKeys_[2 * (nextKeyIndex_ + 1)],
                                 &memory_[0],
                                 &memory_[2]);
        }
    }
}


void scheduleDecryptionRoundKeys(
        void *restrict roundKeys,
        const void *restrict key,
        void *restrict memory
) {
    uint64_t *roundKeys_ = roundKeys;
    uint64_t cache_[2] = {0};

    scheduleEncryptionRoundKeys(roundKeys, key, memory);

    for (int roundKeyIndex_ = 1; roundKeyIndex_ <= 8; ++roundKeyIndex_) {
        memcpy(cache_,
               &roundKeys_[2 * roundKeyIndex_],
               BlockLengthInBytes);
        applySTransformation((unsigned char *) cache_);
        applyInversedLSTransformation((unsigned char *) cache_,
                                      &roundKeys_[2 * roundKeyIndex_]);
    }
}


void encryptBlock(
        const void *restrict roundKeys,
        void *restrict data
) {
    uint64_t *data_ = data;
    const uint64_t *roundKeys_ = roundKeys;
    uint64_t cache_[2] = {0};
    size_t round_ = 0;

    for (; round_ < NumberOfRounds - 1; ++round_) {
        cache_[0] = data_[0] ^ roundKeys_[2 * round_];
        cache_[1] = data_[1] ^ roundKeys_[2 * round_ + 1];

        applyLSTransformation((unsigned char *) cache_, data);
    }

    data_[0] = data_[0] ^ roundKeys_[2 * round_];
    data_[1] = data_[1] ^ roundKeys_[2 * round_ + 1];
}


void decryptBlock(
        const void *restrict roundKeys,
        void *restrict data
) {
    uint64_t *data_ = data;
    const uint64_t *roundKeys_ = roundKeys;
    uint64_t cache_[2] = {0};
    size_t round_ = NumberOfRounds - 1;

    data_[0] ^= roundKeys_[2 * round_];
    data_[1] ^= roundKeys_[2 * round_ + 1];
    --round_;

    applySTransformation(data);
    applyInversedLSTransformation((unsigned char *) data, cache_);
    applyInversedLSTransformation((unsigned char *) cache_, data);

    cache_[0] = data_[0] ^ roundKeys_[2 * round_];
    cache_[1] = data_[1] ^ roundKeys_[2 * round_ + 1];
    --round_;

    for (; round_ > 0; --round_) {
        applyInversedLSTransformation((unsigned char *) cache_, data);

        cache_[0] = data_[0] ^ roundKeys_[2 * round_];
        cache_[1] = data_[1] ^ roundKeys_[2 * round_ + 1];
    }

    applyInversedSTransformation((unsigned char *) cache_);

    data_[0] = cache_[0] ^ roundKeys_[2 * round_];
    data_[1] = cache_[1] ^ roundKeys_[2 * round_ + 1];
}


#else
typedef void ISOCompilerHappiness_t;
#endif
