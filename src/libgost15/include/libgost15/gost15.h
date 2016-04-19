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

enum operationMode_t {
    ECB,
    CBC,
    CFB,
    OFB
};

extern const size_t WorkspaceOfScheduleRoundKeys;

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

#endif
