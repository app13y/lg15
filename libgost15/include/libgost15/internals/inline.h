#pragma once

#include <libgost15/internals/compiler.h>
#include <libgost15/internals/language.h>

/* Deals with MSVC not supporting `inline` keyword in C mode:
 *   https://stackoverflow.com/a/2765211
 *
 * Note that in C++ mode keywords macroising is forbidden:
 *   keywords: xkeycheck.h, error C1189.
 */
#if COMPILER_IS_MSVC && !LANGUAGE_IS_CXX
     #define inline __inline

#endif
