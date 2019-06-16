#pragma once

#include <libgost15/internals/compiler.h>

/* Types declared with this attribute are assumed to alias everything,
 * same as `char`.
 * 
 * `may_alias` works around strict aliasing rules when type-punning is required
 * and `-fstrict-aliasing` and/or `-Wstrict-aliasing` are on.
 *
 * Use with *only when necessary*, this *will slow your code down*.
 */
#if COMPILER_IS_GCC || COMPILER_IS_CLANG
     #define _May_alias \
          __attribute__((__may_alias__))

#else
     #define _May_alias

#endif
