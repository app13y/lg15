#pragma once

#include <libgost15/internals/unmangled.h>
#include <libgost15/internals/restrict.h>
#include <stddef.h>
#include <stdint.h>

enum {
    NumberOfRounds = 10,
    NumberOfRoundsInKeySchedule = 8,

    BlockLengthInBytes = 128 / 8,
    KeyLengthInBytes = 256 / 8,
};

_Unmangled void
lg15_encryptBlocks(
    uint8_t const * restrict roundKeys,
    uint8_t * restrict blocks,
    size_t numberOfBlocks
);

_Unmangled void
lg15_decryptBlocks(
    uint8_t const * restrict roundKeys,
    uint8_t * restrict blocks,
    size_t numberOfBlocks
);

_Unmangled void
lg15_scheduleEncryptionRoundKeys(
    uint8_t * restrict roundKeys,
    uint8_t const * restrict key
);

_Unmangled void
lg15_scheduleDecryptionRoundKeys(
    uint8_t * restrict roundKeys,
    uint8_t const * restrict key
);
