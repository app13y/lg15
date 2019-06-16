#include <libgost15/libgost15.h>
#include <SIMD/SIMD_tables.h>
#include <shared/tables.h>
#include <emmintrin.h>
#include <string.h>

// TODO: Beat alignment warnings with Clang's and GCC's -Wcast-align.

const size_t WorkspaceOfScheduleRoundKeys = 0;

static void applySTransformation(__m128i *block) {
    union block_t {
        uint8_t asBytes[BlockLengthInBytes];
        __m128i asXMMWord;
    } temporary_ = {{0}};
    _mm_storeu_si128(&(temporary_.asXMMWord), *block);
    for (int byteIndex_ = 0; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        temporary_.asBytes[byteIndex_] = Pi[temporary_.asBytes[byteIndex_]];
    }
    *block = _mm_loadu_si128(&temporary_.asXMMWord);
}

static void applyInversedSTransformation(__m128i *block) {
    union block_t {
        uint8_t asBytes[BlockLengthInBytes];
        __m128i asXMMWord;
    } temporary_ = {{0}};
    _mm_storeu_si128(&temporary_.asXMMWord, *block);
    for (int byteIndex_ = 0; byteIndex_ < BlockLengthInBytes; ++byteIndex_) {
        temporary_.asBytes[byteIndex_] = InversedPi[temporary_.asBytes[byteIndex_]];
    }
    *block = _mm_loadu_si128(&temporary_.asXMMWord);
}


static void swapBlocks(
        __m128i *left,
        __m128i *right) {
    *left = _mm_xor_si128(*left, *right);
    *right = _mm_xor_si128(*left, *right);
    *left = _mm_xor_si128(*left, *right);
}


static void applyLSTransformation(
        const __m128i *input,
        __m128i *output
) {
    __m128i temporary1_, temporary2_;
    __m128i addresses1_, addresses2_;

    addresses1_ = _mm_and_si128(*(const __m128i *) bitmask, *input);
    addresses2_ = _mm_andnot_si128(*(const __m128i *) bitmask, *input);

    addresses1_ = _mm_srli_epi16(addresses1_, 4);
    addresses2_ = _mm_slli_epi16(addresses2_, 4);

    temporary1_ = _mm_load_si128((const void *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 0) + 0x1000));
    temporary2_ = _mm_load_si128((const void *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 0)));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable+ _mm_extract_epi16(addresses1_, 1)+ 0x3000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 1) + 0x2000));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 2) + 0x5000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 2) + 0x4000));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 3) + 0x7000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 3) + 0x6000));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 4) + 0x9000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 4) + 0x8000));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 5) + 0xB000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 5) + 0xA000));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 6) + 0xD000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 6) + 0xC000));

    temporary1_ = _mm_xor_si128(temporary1_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses1_, 7) + 0xF000));
    temporary2_ = _mm_xor_si128(temporary2_, *(const __m128i *) (precomputedLSTable + _mm_extract_epi16(addresses2_, 7) + 0xE000));

    *output = _mm_xor_si128(temporary1_, temporary2_);
}


static void applyInversedLSTransformation(
        const __m128i *input,
        __m128i *output
) {
    __m128i cache1_, cache2_;
    __m128i addresses1_, addresses2_;

    addresses1_ = _mm_and_si128(*(const __m128i *) bitmask, *input);
    addresses2_ = _mm_andnot_si128(*(const __m128i *) bitmask, *input);

    addresses1_ = _mm_srli_epi16(addresses1_, 4);
    addresses2_ = _mm_slli_epi16(addresses2_, 4);

    cache1_ = _mm_load_si128((const void *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 0) + 0x1000));
    cache2_ = _mm_load_si128((const void *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 0)));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 1) + 0x3000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 1) + 0x2000));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 2) + 0x5000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 2) + 0x4000));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 3) + 0x7000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 3) + 0x6000));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 4) + 0x9000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 4) + 0x8000));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 5) + 0xB000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 5) + 0xA000));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 6) + 0xD000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 6) + 0xC000));

    cache1_ = _mm_xor_si128(cache1_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses1_, 7) + 0xF000));
    cache2_ = _mm_xor_si128(cache2_, *(const __m128i *) (precomputedInversedLSTable + _mm_extract_epi16(addresses2_, 7) + 0xE000));

    *output = _mm_xor_si128(cache1_, cache2_);
}


static void applyFTransformation(
        int constantIndex,
        __m128i *left,
        __m128i *right
) {
    __m128i temporary1_, temporary2_;

    temporary1_ = _mm_loadu_si128((const __m128i *) &roundConstants[BlockLengthInBytes * constantIndex]);
    temporary2_ = _mm_xor_si128(*left, temporary1_);
    applyLSTransformation(&temporary2_, &temporary1_);
    *right = _mm_xor_si128(*right, temporary1_);
    swapBlocks(left, right);
}


void scheduleEncryptionRoundKeysForGost15(
        void *restrict roundKeys,
        const void *restrict key,
        void *restrict memory
) {
    (void) memory;
    uint64_t *roundKeys_ = roundKeys;

    memcpy(&roundKeys_[0], key, BlockLengthInBytes * 2);

    for (int nextKeyIndex_ = 2, constantIndex_ = 0; nextKeyIndex_ != NumberOfRounds; nextKeyIndex_ += 2) {
        __m128i nextKey1_, nextKey2_;

        nextKey1_ = _mm_loadu_si128((__m128i *) &roundKeys_[2 * (nextKeyIndex_ - 2)]);
        nextKey2_ = _mm_loadu_si128((__m128i *) &roundKeys_[2 * (nextKeyIndex_ - 1)]);

        for (int feistelRoundIndex_ = 0; feistelRoundIndex_ < NumberOfRoundsInKeySchedule; ++feistelRoundIndex_) {
            applyFTransformation(constantIndex_++, &nextKey1_, &nextKey2_);
        }

        _mm_storeu_si128((__m128i *) &roundKeys_[2 * (nextKeyIndex_)], nextKey1_);
        _mm_storeu_si128((__m128i *) &roundKeys_[2 * (nextKeyIndex_ + 1)], nextKey2_);
    }
}


void scheduleDecryptionRoundKeysForGost15(
        void *restrict roundKeys,
        const void *restrict key,
        void *restrict memory
) {
    uint64_t *roundKeys_ = roundKeys;
    scheduleEncryptionRoundKeysForGost15(roundKeys, key, memory);

    for (int keyIndex_ = 1; keyIndex_ <= 8; ++keyIndex_) {
        __m128i temporary1_, temporary2_;

        temporary1_ = _mm_loadu_si128((__m128i *) &roundKeys_[2 * keyIndex_]);
        applySTransformation(&temporary1_);
        applyInversedLSTransformation(&temporary1_, &temporary2_);
        _mm_storeu_si128((__m128i *) &roundKeys_[2 * keyIndex_], temporary2_);
    }
}


void encryptBlockWithGost15(
        const void *restrict roundKeys,
        void *restrict data
) {
    const __m128i *roundKeys_ = roundKeys;
    __m128i data_;
    int round_;

    data_ = _mm_loadu_si128(data);
    for (round_ = 0; round_ < NumberOfRounds - 1; ++round_) {
        __m128i buffer = _mm_xor_si128(data_, roundKeys_[round_]);
        applyLSTransformation(&buffer, &data_);
    }
    data_ = _mm_xor_si128(data_, roundKeys_[round_]);
    _mm_store_si128(data, data_);
}


void decryptBlockWithGost15(
        const void *restrict roundKeys,
        void *restrict data
) {
    const __m128i *roundKeys_ = roundKeys;
    __m128i cache_, data_;

    data_ = _mm_loadu_si128(data);
    data_ = _mm_xor_si128(data_, roundKeys_[NumberOfRounds - 1]);

    applySTransformation(&data_);
    applyInversedLSTransformation(&data_, &cache_);
    applyInversedLSTransformation(&cache_, &data_);
    cache_ = _mm_xor_si128(data_, roundKeys_[NumberOfRounds - 2]);

    for (int round_ = NumberOfRounds - 3; round_ > 0; --round_) {
        applyInversedLSTransformation(&cache_, &data_);
        cache_ = _mm_xor_si128(data_, roundKeys_[round_]);
    }

    applyInversedSTransformation(&cache_);
    data_ = _mm_xor_si128(cache_, roundKeys_[0]);
    _mm_store_si128(data, data_);
}
