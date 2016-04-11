#ifndef GOSTHOPPER_HEADER_INCLUDED_
#define GOSTHOPPER_HEADER_INCLUDED_

#include <stdint.h>
#include <stddef.h>


enum {
    NumberOfRounds = 10,
    NumberOfRoundsInKeySchedule = 8,

    BlockLengthInBytes = 128 / 8,
    KeyLengthInBytes = 256 / 8,

    BlockLengthInLimbs = BlockLengthInBytes / sizeof(uint64_t),
    KeyLengthInLimbs = KeyLengthInBytes / sizeof(uint64_t)
};

enum operationMode_t {
    ECB,
    CBC,
    CFB,
    OFB
};

extern const size_t WorkspaceOfScheduleEncryptionRoundKeys;
extern const size_t WorkspaceOfScheduleDecryptionRoundKeys;
extern const size_t WorkspaceOfEncryptBlock;
extern const size_t WorkspaceOfDecryptBlock;


void encryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
);
void decryptBlock(
        const void *roundKeys,
        void *block,
        void *memory
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
