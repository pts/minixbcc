/* More compatible replacement for the libc <stdlib.h> header. */

#ifndef _MYSLIB_H
#define _MYSLIB_H

#ifdef NOSTDLIBH  /* For systems for which `#include <stdio.h>' causes a compile error. */
  /* Example system: Minix 1.5.10 i86 with /usr/include containing `div_t div(...)', compiling with BCC sc (from e.g. bccbin16.tar.Z). */
#  include <stddef.h>  /* For NULL and size_t. */
#  ifdef __STDC__
#    define _MYSLIB_PROTOTYPE(function, params) function params
#  else
#    define _MYSLIB_PROTOTYPE(function, params) function()
#  endif
  /*_MYSLIB_PROTOTYPE(void abort, (void));*/
  _MYSLIB_PROTOTYPE(void exit, (int _status));
  _MYSLIB_PROTOTYPE(void *malloc, (size_t _size));
  _MYSLIB_PROTOTYPE(void *realloc, (void *_ptr, size_t _size));
  _MYSLIB_PROTOTYPE(void free, (void *_ptr));
#else
#  include <stdlib.h>
#endif

#endif /* _MYSLIB_H */
