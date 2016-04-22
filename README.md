## libgost15
[![Build Status](https://travis-ci.org/aprelev/libgost15.svg?branch=master)](https://travis-ci.org/aprelev/libgost15)
[![GitHub release](https://img.shields.io/github/release/aprelev/libgost15.svg?maxAge=2592000)]()

Year 2015 introduced new national block cipher standard GOST 34.12 '15, which goes by codename Grasshopper (**Kuznechik** as in **Kuz**min, **Nech**aev and **K**ompany).

New cipher itself has a structure of SP-network with fixed byte-to-byte S-box and linear transformations over Galois Field GF(256). It features

* increased block length (128 bits),
* decreased number of rounds (10 rounds),
* advanced round keys scheduling routine (Feistel network)

compared to its predecessor GOST 28147.

This repository hosts C99 library `libgost15` which provides three (interchangeable) versions of basic implementations:

* compact implementation,
* optimised implementation,
* SIMD implementation.

### Performance

Performance is measured by a separate tool residing in benchmark subdirectory. It links with `libgost15` and measures speed of these operations:

* block encryption,
* block decryption.

All functions provided by `libgost15` are thread-safe thus measuring takes place in single thread.

#### Benchmark data (Intel Core i7-2600 Sandy Bridge @ 3.40 GHz, single core)

| Operation        | `compact`   | `optimised`   | `SIMD`        |
|:---------------- |:----------- |:------------- |:------------- |
| Block encryption | 4.4321 MB/s | 100.8338 MB/s | 158.8720 MB/s |
| Block decryption | 4.3837 MB/s | 102.0845 MB/s | 157.5190 MB/s |

#### Benchmark data (Intel Core i7-2677M Sandy Bridge @ 1.80 GHz, single core)

| Operation        | `compact`   | `optimised`   | `SIMD`        |
|:---------------- |:----------- |:------------- |:------------- |
| Block encryption | 1.2840 MB/s |  62.6575 MB/s | 112.2875 MB/s |
| Block decryption | 1.2676 MB/s |  64.4036 MB/s | 114.6625 MB/s |


### Implementations

#### Compact implementation

Straightforward implementation of block encryption and decryption routines, with little or no major optimisations. Has lowest memory requirements. Does not require SSE instructions.

Why use this and not [official TC26 implementation](http://tc26.ru/standard/gost/PR_GOSTR_bch_v6.zip)?

* It works on any platform, not just Windows.
* All sixteen R transformations are merged into single L transformation thus cutting out rotations.
* Better grammar and code organisation.

This implementation is build by default and it does not require any special predefined variables.

#### Optimised implementation

Optimised implementation employs vector-by-matrix multiplication precomutation technique described in [no link yet], similar to one in 64KB versions of AES. This implementation is much faster that the compact one, but requires 128KB os additional memory in data segment for storing precomputed tables. Does not require SSE instructions.

To use optimised implementation, define `ENABLE_PRECALCULATIONS` environment variable before building:

```
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_PRECALCULATIONS=ON ...
```

#### SIMD implementation

SIMD implementation utilises SSE instruction set, a set of extended processor instructions which enable one to operate over 128-bit XMM registers, thus further speeding up optimised implementation. Requires SSE2 or higher.

To use optimised implementation, define both `ENABLE_PRECALCULATIONS` and `ENABLE_SIMD` environment variables before building:

```
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_PRECALCULATIONS=ON -DENABLE_SIMD=ON ...
```

Future versions of `libgost15` might enable this implementation version by default when optimised version is selected and SSE instruction set (SSE2+) is available.

### Portability

I am working as hard as I can to make this code portable and test it on as many platforms as I can. You are welcome to contribute.
