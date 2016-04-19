#include <stdlib.h>
#include <stdio.h>
#include <libgost15/gost15.h>

#if defined __APPLE__
    #include <mach/mach_time.h>
#elif defined COMPILER_MSVC && !defined _M_IX86
    extern "C" uint64_t __rdtsc();
#elif !defined _WIN32
    #include <sys/time.h>
#endif


int64_t getCyclesCount() {
#if defined __APPLE__
    return mach_absolute_time();
#elif defined __i386__
    int64_t ret;
    __asm__ volatile("rdtsc" : "=A"(ret));
    return ret;
#elif defined __x86_64__ || defined __amd64__
    uint64_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return (high << 32) | low;
#elif defined __powerpc__ || defined __ppc__
    /* This returns a time-base, which is not always precisely a cycle-count. */
    int64_t tbl, tbu0, tbu1;
    asm("mftbu %0" : "=r"(tbu0));
    asm("mftb  %0" : "=r"(tbl));
    asm("mftbu %0" : "=r"(tbu1));
    tbl &= -static_cast<int64>(tbu0 == tbu1);
    return (tbu1 << 32) | tbl;
#elif defined __sparc__
    int64_t tick;
    asm(".byte 0x83, 0x41, 0x00, 0x00");
    asm("mov   %%g1, %0" : "=r"(tick));
    return tick;
#elif defined __ia64__
    int64_t itc;
    asm("mov %0 = ar.itc" : "=r"(itc));
    return itc;
#elif defined COMPILER_MSVC && defined _M_IX86
    /* Older MSVC compilers (like 7.x) don't seem to support the
       __rdtsc intrinsic properly, so _asm usage is preferred instead. */
    _asm rdtsc
#elif defined COMPILER_MSVC
    return __rdtsc();
#elif defined __aarch64__
    int64_t virtual_timer_value;
    asm volatile("mrs %0, cntvct_el0" : "=r"(virtual_timer_value));
    return virtual_timer_value;
#elif defined __ARM_ARCH
#if (__ARM_ARCH >= 6)  /* V6 is the earliest arch that has a standard cyclecount. */
        uint32_t pmccntr;
        uint32_t pmuseren;
        uint32_t pmcntenset;
        /* Read the user mode perf monitor counter access permissions. */
        asm("mrc p15, 0, %0, c9, c14, 0" : "=r"(pmuseren));
        if (pmuseren & 1) {  /* Allows reading perfmon counters for user mode code? */
            asm("mrc p15, 0, %0, c9, c12, 1" : "=r"(pmcntenset));
            if (pmcntenset & 0x80000000ul) {  /* Is it counting? */
                asm("mrc p15, 0, %0, c9, c13, 0" : "=r"(pmccntr));
                /* The counter is set up to count every 64th cycle. */
                return static_cast<int64_t>(pmccntr) * 64;
            }
        }
#endif
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#elif defined __mips__
    /* mips only allows rdtsc for superusers */
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<int64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#else
    #error Define cycle timer for your platform
#endif
}


void benchmarkEncryption(unsigned long iterations) {
    int64_t startCyclesCounter_, finishCyclesCounter_;
    size_t iterationIndex_ = 0;
    const uint8_t roundKeys_[NumberOfRounds * BlockLengthInBytes] = {
            0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
            0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10, 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
            0xdb, 0x31, 0x48, 0x53, 0x15, 0x69, 0x43, 0x43, 0x22, 0x8d, 0x6a, 0xef, 0x8c, 0xc7, 0x8c, 0x44,
            0x3d, 0x45, 0x53, 0xd8, 0xe9, 0xcf, 0xec, 0x68, 0x15, 0xeb, 0xad, 0xc4, 0x0a, 0x9f, 0xfd, 0x04,
            0x57, 0x64, 0x64, 0x68, 0xc4, 0x4a, 0x5e, 0x28, 0xd3, 0xe5, 0x92, 0x46, 0xf4, 0x29, 0xf1, 0xac,
            0xbd, 0x07, 0x94, 0x35, 0x16, 0x5c, 0x64, 0x32, 0xb5, 0x32, 0xe8, 0x28, 0x34, 0xda, 0x58, 0x1b,
            0x51, 0xe6, 0x40, 0x75, 0x7e, 0x87, 0x45, 0xde, 0x70, 0x57, 0x27, 0x26, 0x5a, 0x00, 0x98, 0xb1,
            0x5a, 0x79, 0x25, 0x01, 0x7b, 0x9f, 0xdd, 0x3e, 0xd7, 0x2a, 0x91, 0xa2, 0x22, 0x86, 0xf9, 0x84,
            0xbb, 0x44, 0xe2, 0x53, 0x78, 0xc7, 0x31, 0x23, 0xa5, 0xf3, 0x2f, 0x73, 0xcd, 0xb6, 0xe5, 0x17,
            0x72, 0xe9, 0xdd, 0x74, 0x16, 0xbc, 0xf4, 0x5b, 0x75, 0x5d, 0xba, 0xa8, 0x8e, 0x4a, 0x40, 0x43,
    };
    uint8_t block_[BlockLengthInBytes] = {
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };


    startCyclesCounter_ = getCyclesCount();
    while (iterationIndex_++ < iterations) {
        encryptBlock(roundKeys_, block_);
    }
    finishCyclesCounter_ = getCyclesCount();

    printf("\nBenchmark results:\n");
    printf("%lu bytes processed within %lli cycles with average speed of %04.4f cycles/byte.\n",
           iterations * BlockLengthInBytes,
           finishCyclesCounter_ - startCyclesCounter_,
           (double) (finishCyclesCounter_ - startCyclesCounter_) / (iterations * BlockLengthInBytes));

    return;
}


int main(void) {
    benchmarkEncryption(1024 * 1024 * 16);
    return 0;
}