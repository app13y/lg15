#if !defined LIBGOST15_HEADER_INCLUDED_
#define LIBGOST15_HEADER_INCLUDED_

#include <stdint.h>
#include <stddef.h>


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

void encryptBlock(
        const void *roundKeys,
        void *block
);
void decryptBlock(
        const void *roundKeys,
        void *block
);

void scheduleEncryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
);

void scheduleDecryptionRoundKeys(
        void *roundKeys,
        const void *key,
        void *memory
);

#ifdef __cplusplus
}
#endif

#endif
