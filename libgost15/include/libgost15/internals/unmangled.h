#pragma once

#include <libgost15/internals/language.h>

/* Defines standard export specification for all public declarations
 * which require plain C linkage (this is use for most if not all exported symbols).
 */
#if !defined(_Unmangled)
     #if LANGUAGE_IS_CXX
          #define _Unmangled \
               extern "C"

     #else
          #define _Unmangled \
               extern

     #endif

#endif
