#include <libgost15/libgost15.h>
#include <stdlib.h>
#include <string.h>


int testBlockDecryption(void) {
    int numberOfFailedTests_ = 0;
    void *roundKeys_ = NULL, *memory_ = NULL;

    const uint8_t secretKey_[KeyLengthInBytes] = {
            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
            0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
    };

    uint8_t block_[BlockLengthInBytes] = {
            0x7f, 0x67, 0x9d, 0x90, 0xbe, 0xbc, 0x24, 0x30, 0x5a, 0x46, 0x8d, 0x42, 0xb9, 0xd4, 0xed, 0xcd,
    };

    const uint8_t expectedPlaintext_[BlockLengthInBytes] = {
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x00, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
    };

    roundKeys_ = malloc(NumberOfRounds * BlockLengthInBytes);
    if (!roundKeys_) {
        ++numberOfFailedTests_;
        goto cleanup;
    }

    memory_ = malloc(WorkspaceOfScheduleRoundKeys);
    if (!memory_ && WorkspaceOfScheduleRoundKeys) {
        ++numberOfFailedTests_;
        goto cleanup;
    }

    scheduleDecryptionRoundKeysForGost15(roundKeys_, secretKey_, memory_);
    decryptBlockWithGost15(roundKeys_, block_);
    numberOfFailedTests_ += (memcmp(block_, expectedPlaintext_, BlockLengthInBytes) != 0);

    cleanup:
    free(memory_);
    return numberOfFailedTests_;
}


int main(void) {
    return testBlockDecryption();
}
