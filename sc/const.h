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

/*#define DEBUG*/			/* generate compiler-debugging code */
#ifndef MC6809
#  define I8088			/* target processor is Intel 8088 thru 80386 */
#endif
/*#define MC6809*/		/* target processor is Motorola 6809 */
#define SELFTYPECHECK		/* check calculated type = runtime type */

#ifdef I8088
# define DYNAMIC_LONG_ORDER 1	/* long word order spec. at compile time */
# define FRAMEPOINTER		/* index locals off frame ptr, not stack ptr */
# define HOLDSTRINGS		/* hold strings for dumping at end
				 * since assembler has only 1 data seg */
#endif

#ifdef MC6809
# define DYNAMIC_LONG_ORDER 0	/* have to define it so it works in #if's */
# define OP1			/* logical operators only use 1 byte */
# define POSINDEPENDENT		/* position indep code can (also) be gen */
#endif

#define S_ALIGNMENT (sizeof(int))  /* source memory alignment, power of 2 */

#define UNPORTABLE_ALIGNMENT  /* we would need portable alignment for large model on DOS */

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
