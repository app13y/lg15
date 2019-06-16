/// @file
///
/// @copyright
/// InfoTeCS

#pragma once

#include <libgost15/internals/compiler.h>
#include <libgost15/internals/language.h>

/* restrict is not available in C++, that's a first.
 *
 * None of MSVC toolsets recognise this keyword in C mode,
 * MSDN suggests __restrict specifier.
 * 
 * See https://docs.microsoft.com/en-us/cpp/cpp/extension-restrict?view=vs-2017.
 */

#if LANGUAGE_IS_CXX
     #define restrict

#elif COMPILER_IS_MSVC

     /* MSVC 2013 (v120) toolset is known to break certain code with __restrict. */
     #if _MSC_VER <= 1800
          #define restrict

     #else
          #define restrict \
               __restrict

     #endif
#endif
