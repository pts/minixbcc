/* align.h - memory alignment requirements for linker */

#ifndef S_ALIGNMENT
#  define align(x)
#else
#  if S_ALIGNMENT < 2
#    define align(x)
#  else
#    define align(x) ((x) = (void*)(((int)(void*) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1)))
#  endif
#endif
