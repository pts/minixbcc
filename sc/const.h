/* const.h - constants for bcc */

/* Copyright (C) 1992 Bruce Evans */

/* switches for code generation */

#ifndef NOFP  /* Configurable for the command line. */
#  ifndef FP
#    define NOFP 1  /* Disable floating point support by default, because the libc support functions (such as fadd, Fsub, fadd, fsub) are not implemented, and also to reduce cross-compiler host dependencies. */
#  endif
#endif

#ifdef FP
#  ifdef NOFP
#    error Both FP and NOFP are defined.
#  endif
#endif

#ifndef SMALLMEM  /* Configurable from the command line. */
#  ifndef NOSMALLMEM
#    ifdef __BCC__
#      ifdef __AS386_16__
#        define SMALLMEM 1
#      endif
#    endif
#    ifndef SMALLMEM
#      ifdef __WATCOMC__
#        ifdef _M_I86  /* Not defined for i386 (__386__). */
#          define SMALLMEM 1
#        endif
#      endif
#    endif
#    ifndef SMALLMEM
#      if __SIZEOF_POINTER__ && __SIZEOF_POINTER__ <= 2  /* Recent gcc-ia16 in small model, or manually defined. */
#        define SMALLMEM 1
#      endif
#    endif
#    ifndef SMALLMEM
#      if _EM_PSIZE && _EM_PSIZE <= 2  /* ACK ANSI C compiler 1.202 in Minix >= 1.7.0, when targetin i86. */
#        define SMALLMEM 1
#      endif
#    endif
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
