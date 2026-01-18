/* align.h - memory alignment requirements for linker */

/* align(x) works on char *x. */
#ifndef S_ALIGNMENT
#  define align(x)
#else
#  if S_ALIGNMENT < 2
#    define align(x)
#  else
#    ifdef BCCALIGNFIX
      typedef char assert_alignptrsize[sizeof(unsigned) >= sizeof(void*)];  /* True for both BCC i86 (bcc -0) and BCC i386 (bcc -3). */
#      define align(x) ((x) = (char*)(((unsigned) (void*) (x) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1)))  /* This assumes sizeof(unsigned) >= sizeof(char*). */
#    else
#      define align(x) ((x) += -(unsigned) (x) & (unsigned) ((S_ALIGNMENT) - 1))  /* This works even if sizeof(char*) > sizeof(unsigned). */  /* BCC sc v0 (1990-06-09) generates incorrect code for this for both i86 (bcc -0) and i386 (bcc -3). */
#    endif
#  endif
#endif
