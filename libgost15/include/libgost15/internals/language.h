/// @file
///
/// @copyright
/// InfoTeCS

#pragma once

#include <libgost15/internals/compiler.h>

#if defined __cplusplus
     #define LANGUAGE_IS_CXX 1

     #if (__cplusplus >= 201703L) || COMPILER_IS_MSVC && (_MSVC_LANG > 201703L)
          #define LANGUAGE_IS_CXX17 1

     #elif (__cplusplus >= 201402L) || COMPILER_IS_MSVC && (_MSVC_LANG > 201402L)
          #define LANGUAGE_IS_CXX14 1

     #elif (__cplusplus >= 201103L) || COMPILER_IS_MSVC && (_MSVC_LANG > 201103L)
          #define LANGUAGE_IS_CXX11 1

     #elif (__cplusplus >= 199711L) || COMPILER_IS_MSVC && (_MSVC_LANG > 199711L)
          #define LANGUAGE_IS_CXX98 1

     #else
          #define LANGUAGE_IS_UNKNOWN 1

     #endif

#elif defined __STDC__
     #define LANGUAGE_IS_C 1

     #if defined __STDC_VERSION__
          #if (__STDC_VERSION__ >= 201112L) || defined _ISOC11_SOURCE
               #define LANGUAGE_IS_C11 1

          #elif (__STDC_VERSION__ >= 199901L) || defined _ISOC99_SOURCE
               #define LANGUAGE_IS_C99 1

          #else
               #define LANGUAGE_IS_UNKNOWN 1

          #endif

     #else
          #define LANGUAGE_IS_C89 1

     #endif

#endif


#if !defined LANGUAGE_IS_CXX
     #define LANGUAGE_IS_CXX 0
#endif

#if !defined LANGUAGE_IS_CXX14
     #define LANGUAGE_IS_CXX14 0
#endif

#if !defined LANGUAGE_IS_CXX11
     #define LANGUAGE_IS_CXX11 0
#endif

#if !defined LANGUAGE_IS_CXX17
     #define LANGUAGE_IS_CXX17 0
#endif

#if !defined LANGUAGE_IS_CXX98
     #define LANGUAGE_IS_CXX98 0
#endif

#if !defined LANGUAGE_IS_C
     #define LANGUAGE_IS_C 0
#endif

#if !defined LANGUAGE_IS_C11
     #define LANGUAGE_IS_C11 0
#endif

#if !defined LANGUAGE_IS_C99
     #define LANGUAGE_IS_C99 0
#endif

#if !defined LANGUAGE_IS_C89
     #define LANGUAGE_IS_C89 0
#endif

#if !defined LANGUAGE_IS_UNKNOWN
     #define LANGUAGE_IS_UNKNOWN 0
#endif