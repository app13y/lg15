#if !defined LIBGOST15_HEADER_INCLUDED_
#define LIBGOST15_HEADER_INCLUDED_

#include <libgost15/platform.h>

enum {
    NumberOfRounds = 10,
    NumberOfRoundsInKeySchedule = 8,

    BlockLengthInBytes = 128 / 8,
    KeyLengthInBytes = 256 / 8,
};

extern const size_t WorkspaceOfScheduleRoundKeys;

#ifdef __cplusplus
extern "C" {
#endif

void encryptBlockWithGost15(
        const void *roundKeys,
        void *block
);

void decryptBlockWithGost15(
        const void *roundKeys,
        void *block
);

void scheduleEncryptionRoundKeysForGost15(
        void *roundKeys,
        const void *key,
        void *memory
);

void scheduleDecryptionRoundKeysForGost15(
        void *roundKeys,
        const void *key,
        void *memory
);

#ifdef __cplusplus
}
#endif

#endif
