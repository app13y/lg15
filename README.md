> Released version 0.3.5

### Description

Year 2015 introduced new national block cipher standard GOST 34.12 '15, which goes by codename Grasshopper (**Kuznechik** as in **Kuz**min, **Nech**aev and **K**ompany).

New cipher itself has a structure of SP-network with fixed byte-to-byte S-box and linear transformations over Galois Field GF(256). It features

* increased block length (128 bits),
* decreased number of rounds (10 rounds),
* advanced round keys scheduling routine (Feistel network)

compared to its predecessor GOST 28147.

This project provides several C99 versions of implementation with minimal or no dependencies while achieving high performance:

* compact implementation,
* optimised implementation,
* SIMD implementation.

#### Compact implementation

Straightforward implementation of block encryption and decryption routines, with little or no major optimisations. Has lowest memory requirements.

#### Optimised implementation

To use optimised implementation, define `USE_OPTIMISED_IMPLEMENTATION` environment variable before compiling.

Optimised implementation employs vector-by-matrix multiplication precomutation technique described in [add link], similar to one in 64KB versions of AES. This implementation is much faster that the compact one, but requires 128KB os additional memory in data segment for storing precomputed tables.

#### SIMD implementation

SIMD implementation automatically enables when `USE_OPTIMISED_IMPLEMENTATION` is defined and Intel (at least) SSE2 instruction set is supported by processor.

SIMD implementation utilises SSE instruction set, a set of extended processor instructions which enable one to operate over 128-bit XMM registers. Combined with vector-by-matrix multiplication, SSE instructions help to achieve incredible performance.

### Portability

Source code is by no means portable on all platforms out-of-the-box, though it should be fairly easy to port compact version of implementation on any platform with a few minor tweaks. 

Porting optimised and SIMD versions on platform with a different endianness requires rotating each vector in precalculated long tables. 
