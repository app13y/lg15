#pragma once

#if !defined COMPILER_IS_DETECTED
     #if defined(__clang__)
          #define COMPILER_IS_CLANG 1

     #elif defined(__GNUC__)
          #define COMPILER_IS_GCC 1

     #elif defined(_MSC_VER)
          #define COMPILER_IS_MSVC 1

     #elif defined(__INTEL_COMPILER) || defined(__ICC) || defined(__ECC) || defined(__ICL)
          #define COMPILER_IS_ICC 1

     #else
          #define COMPILER_IS_UNKNOWN 1

     #endif
#endif

#if !defined COMPILER_IS_CLANG
     #define COMPILER_IS_CLANG 0
#endif

#if !defined COMPILER_IS_GCC
     #define COMPILER_IS_GCC 0
#endif

#if !defined COMPILER_IS_MSVC
     #define COMPILER_IS_MSVC 0
#endif

#if !defined COMPILER_IS_ICC
     #define COMPILER_IS_ICC 0
#endif

#if !defined COMPILER_IS_UNKNOWN
     #define COMPILER_IS_UNKNOWN 0
#endif