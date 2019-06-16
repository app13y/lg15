#pragma once

#include <libgost15/internals/compiler.h>
#include <libgost15/internals/language.h>

/* `_Alignas` attribute may be applied to declaration of a variable to
 * specify an alignment requirement.
 */
#if LANGUAGE_IS_CXX11 || LANGUAGE_IS_CXX14 || LANGUAGE_IS_CXX17
     /* We do nothing, `alignas` is a keyword. */

#elif LANGUAGE_IS_C11
     /* We can use `alignas` defined in <stdalign.h>. */
     #include <stdalign.h>

#else
     /* We define `alignas` manually. */
     #if !defined(_Alignas)
          #if COMPILER_IS_MSVC
               #define _Alignas(boundary) \
                    __declspec(align(boundary))

          #elif COMPILER_IS_GCC || COMPILER_IS_CLANG || COMPILER_IS_ICC
               #define _Alignas(boundary) \
                    __attribute__((__aligned__(boundary)))

          #endif
     #endif

     /* Defines `alignas` convenience macro. */
     #if !defined(alignas)
          #define alignas _Alignas
          #define __alignas_is_defined 1

     #endif
#endif
