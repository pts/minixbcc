/* align.h - memory alignment requirements for the compiler backend */

/* align(x) works on any pointer x, and it returns a (char *). */
#if SC_ALIGNMENT < 2
#  define align(x) ((char *) (x))
#else
  typedef char assert_alignptrsize[sizeof(INTPTRT) >= sizeof(char *)];
#  ifdef PORTALIGN  /* This portable alignment implementation avoids arithmetic on integer casted from pointers. It is needed on e.g. the DOS large model. It is longer than the non-portable implementation. */
    /* The `(int) (INTPTRT)' cast is to pacify the GCC warning -Wpointer-to-int-cast. */
#    define align(x) ((char *) (x) + (-(int) (INTPTRT) (x) & (SC_ALIGNMENT - 1)))  /* This works in BCC sc v0 and v3. However, PORTALIGN is typically disabled for BCC. */
#  else
#    define align(x) ((char *) (((INTPTRT) (char *) (x) + (SC_ALIGNMENT - 1)) & ~(SC_ALIGNMENT - 1)))
#  endif
#endif

extern uoffset_t alignmask;	/* general alignment mask */
