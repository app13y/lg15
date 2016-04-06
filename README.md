> Project is now under development.

### Description

Year 2015 introduced new national block cipher standard GOST 34.12 '15, which goes by codename Grasshopper (**Kuznechik** as in **Kuz**min, **Nech**aev and **K**ompany).

New cipher itself has a structure of SP-network with fixed byte-to-byte S-box and linear transformations over Galois Field GF(256). It features

* increased block length (128 bits),
* decreased number of rounds (10 rounds),
* advanced round keys scheduling routine (Feistel network)

compared with its predecessor GOST 28147.

This project provides several ANSI/ISO C versions of implementation with minimal or no dependencies while achieving high performance:

* compact implementation,
* optimised implementation (*yet to be implemented*),
* SIMD implementation (*yet to be implemented*).

#### Compact implementation

Straightforward implementation of block encryption and decryption routines, with little or no major optimisations. Has lowest memory requirements.


### Portability

Source code is by no means portable on all platforms out-of-the-box, though it should be fairly easy to port compact version of implementation on any platform with a few minor tweaks. 

Porting optimised and SIMD versions on platform with a different endianness requires rotating each vector in precalculated long tables. 


### Compiling

Project configuration is currently a work-in-progress, sources are currently compiled with

```
-ansi -pedantic -Wall -Wextra -Werror -Wno-unused-function
```

