/* const.h - constants for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* switches for code generation */

#ifndef NOFP
#  ifndef FP
#    define NOFP 1  /* Disable floating point support by default, because the libc support functions (such as fadd, Fsub, fadd, fsub) are not implemented, and also to reduce cross-compiler host dependencies. */
#  endif
#endif

#ifdef FP
#  ifdef NOFP
#    error Both FP and NOFP are defined.
#  endif
#endif

/*#define DEBUG*/		/* generate compiler-debugging code */
#define SELFTYPECHECK		/* check calculated type = runtime type */

#define FRAMEPOINTER		/* index locals off frame ptr, not stack ptr */
#define HOLDSTRINGS		/* hold strings for dumping at end
				 * since assembler has only 1 data seg */

/* local style */

#define FALSE 0
#define TRUE 1

#ifndef EXTERN
#  define EXTERN extern
#endif
#ifndef FORWARD
#  define FORWARD static
#endif
#ifndef PRIVATE
#  define PRIVATE static
#endif
#ifndef PUBLIC
#  define PUBLIC
#endif
