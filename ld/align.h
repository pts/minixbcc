/* align.h - memory alignment requirements for linker */

#ifndef LD_ALIGNMENT
#  define LD_ALIGNMENT ALIGNBYTES  /* host memory alignment, power of 2 */
#endif

/* align_add(x) works on char *x. */
#if LD_ALIGNMENT < 2
#  define align_add(x)
#else
  typedef char assert_alignptrsize[sizeof(INTPTRT) >= sizeof(char *)];
#  ifdef PORTALIGN  /* This portable alignment implementation avoids arithmetic on integer casted from pointers. It is needed on e.g. the DOS large model. It is more code than the non-portable implementation. */
#    if 1  /* The `(int) (INTPTRT)' cast is to pacify the GCC warning -Wpointer-to-int-cast. */
#      define align_add(x)     ((x) += -(int)      (INTPTRT) (x) &            (LD_ALIGNMENT - 1))  /* This works in BCC sc v0 and v3. However, PORTALIGN is typically disabled for BCC. */
#    else
#      define align_add_bad(x) ((x) += -(unsigned) (INTPTRT) (x) & (unsigned) (LD_ALIGNMENT - 1))  /* This is buggy in BCC sc v0 (1990-06-09), but not in v3, both `sc -0' and `sc -3': it applies the `&' only to the low byte. */
#    endif
#  else
#    define align_add(x) ((x) = (char *) (((INTPTRT) (char *) (x) + (LD_ALIGNMENT - 1)) & ~(LD_ALIGNMENT - 1)))
#  endif
#endif
