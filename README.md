## lg15

[![Travis](https://travis-ci.org/aprelev/libgost15.svg?branch=master)](https://travis-ci.org/aprelev/libgost15) [![Codecov](https://codecov.io/gh/aprelev/libgost15/branch/master/graph/badge.svg)](https://codecov.io/gh/aprelev/libgost15) 
[![GitHub release](https://img.shields.io/github/release/aprelev/libgost15.svg?maxAge=2592000)](https://github.com/aprelev/libgost15/releases/latest)

New Russian national block cipher GOST R 34.12-'15,
which goes by codename **Kuznechik** (as in **Kuz**min, **Nech**aev and **K**ompany),
was introduced in 2015.

New cipher itself has a structure of SP-network
with fixed byte-to-byte S-box and
linear transformations over Galois Field GF(256).
It features

* increased block length (128 bits),
* decreased number of rounds (10 rounds),
* advanced round keys scheduling routine (Feistel network)

compared to its predecessor GOST 28147.

`lg15` library provides implementation for
fast block encryption, decryption, and round keys scheduling by
employing vector-by-matrix multiplication precomutation technique described in [no link yet],
similar to one in 64KB versions of AES.
This optimisation provides significant speed-up,
but requires 128KB of additional memory for storing precomputed tables.

Two (interchangeable) versions of implementations are provided:

* `Universal` implementation, which is written in pure C, and
* `SSE2` implementation, which utilises SSE2 instructions.

### Tests

Configure with `WITH_TESTS` to build `tests` executables,
which evaluates implementation of encryption, decryption and keys scheduling against data from specification.

Tests can be run via CTest.

### Benchmarks

Configure with `WITH_BENCHMARKS` to build `benchmarks` executable.
All functions provided by `lg15` are non-blocking thus measuring takes place in single thread.

### Portability

I am working as hard as I can to make this code portable and test it on as many platforms as I can.
Bug reports and pull requests are welcome.
