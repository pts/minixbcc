#ifndef _AUTOC_H  /* Autodetects INT32T, INTPTRT, ALIGNBYTES and NOPORTALIGN if needed. */
#define _AUTOC_H 1

#ifdef __GNUC__  /* Also true for Clang and PCC (__PCC__). */
#  if defined(__SIZEOF_INT__) || defined(__SIZEOF_LONG__) || defined(__SIZEOF_POINTER__)  /* Since GCC 4.2. */
#    if !defined(__SIZEOF_INT__) || !defined(__SIZEOF_LONG__) || !defined(__SIZEOF_POINTER__)
#      error Only some of __SIZEOF_INT__, __SIZEOF_LONG__ and __SIZEOF_POINTER__ defined.
#    endif
#  else  /* GCC <=4.1.x */
#    if defined(__ILP32__) && defined(__LP64__)
#      error Both __ILP32__ and __LP64__ are defined.
#    endif
#    if !defined(__ILP32__) && !defined(__LP64__)
#      if defined(__sparc_v9__) || (defined(__sparc__) && defined(__arch64__)) || defined(__x86_64__) || defined(__s390x__) || defined(__PPC64__) || defined(__ppc64__) || defined(__ia64__) || defined(__alpha__)
#        define __LP64__ 1
#      else
#        ifdef __mips__
#          if defined(__mips_n64) || defined(__mips64__) || (defined(_MIPS_SIM) && defined(_ABI64) && _MIPS_SIM == _ABI64) || (defined(_MIPS_SZLONG) && _MIPS_SZLONG == 64)
#            define __LP64__ 1
#          endif
#        endif  /* else for GCC <=4.1.x: if defined(__i386__) || defined(__vax__) || defined(__sparc__) || defined(__s390__) || defined(__powerpc__) || defined(__m68k__) || defined(__hppa__) || defined(__arm__) */
#      endif
#      ifndef __LP64__
#        define __ILP32__
#      endif
#    endif  /* Now exactly one of __ILP32__ and __LP64__ is defined. */
#    ifdef __LP64__
#      define __SIZEOF_INT__ 4
#      define __SIZEOF_LONG__ 8
#      define __SIZEOF_POINTER__ 8
#    else  /* ifdef __ILP32__ */
#      define __SIZEOF_INT__ 4
#      define __SIZEOF_LONG__ 4
#      define __SIZEOF_POINTER__ 4
#    endif
#  endif
#endif

#ifdef __BCC__
#  if __AS386_16__ + __AS386_32__
#    undef __SIZEOF_INT__
#    undef __SIZEOF_LONG__
#    undef __SIZEOF_POINTER__
#    ifdef __AS386_32__
#      define __SIZEOF_INT__ 4
#      define __SIZEOF_LONG__ 4
#      define __SIZEOF_POINTER__ 4
#    else  /* ifdef __AS386_16__ */
#      define __SIZEOF_INT__ 2
#      define __SIZEOF_LONG__ 4
#      define __SIZEOF_POINTER__ 2
#    endif
#  endif
#endif

#ifndef INT32T
#  if __SIZEOF_INT__ == 4
#    define INT32T int
#  else
#    if __SIZEOF_LONG__ == 4
#      define INT32T long
#    else
#      error Autodetection failed, please specify -DINT32T=type
#    endif
#  endif
#endif

#ifndef INTPTRT
#  if __SIZEOF_INT__ && __SIZEOF_INT__ == __SIZEOF_POINTER__
#    define INTPTRT int
#  else
#    if __SIZEOF_LONG__ && __SIZEOF_LONG__ == __SIZEOF_POINTER__
#      define INTPTRT long
#    else
#      error Autodetection failed, please specify -DINTPTRT=type
#    endif
#  endif
#endif

#ifndef ALIGNBYTES
#  if (__SIZEOF_LONG__ == 32 && __SIZEOF_POINTER__ <= 32) || (__SIZEOF_LONG <= 32 && __SIZEOF_POINTER == 32)
#    define ALIGNBYTES 32
#  else
#    if (__SIZEOF_LONG__ == 16 && __SIZEOF_POINTER__ <= 16) || (__SIZEOF_LONG <= 16 && __SIZEOF_POINTER == 16)
#      define ALIGNBYTES 16
#    else
#      if (__SIZEOF_LONG__ == 8 && __SIZEOF_POINTER__ <= 8) || (__SIZEOF_LONG <= 8 && __SIZEOF_POINTER == 8)
#        define ALIGNBYTES 8
#      else
#        if __SIZEOF_LONG__ == 4 && __SIZEOF_INT__ <= 4 && __SIZEOF_POINTER__ <= 4
#          define ALIGNBYTES 4
#        else
#          error Autodetection failed, please specify -DALIGNBYTES=size
#        endif
#      endif
#    endif
#  endif
#endif

typedef char assert_sizeof_int32t[sizeof(INT32T) == 4 ? 1 : -1];
typedef char assert_sizeof_intptrt[sizeof(INTPTRT) == sizeof(char *) ? 1 : -1];

#ifndef NOPORTALIGN
#  ifdef __BCC__
#    if __AS386_16__ + __AS386_32__
#      define NOPORTALIGN 1
#    endif
#  endif
#endif
#ifndef NOPORTALIGN
#  ifdef __GCC__
#    if defined(__mips__) || defined(__sparc_v9__) || (defined(__sparc__) && defined(__arch64__)) || defined(__x86_64__) || defined(__s390x__) || defined(__PPC64__)
#      define NOPORTALIGN 1
#    else
#      if defined(__ppc64__) || defined(__ia64__) || defined(__alpha__) || defined(__mips_n64) || defined(__mips64__) || defined(__i386__)
#        define NOPORTALIGN 1
#      else
#        if defined(__vax__) || defined(__sparc__) || defined(__s390__) || defined(__powerpc__) || defined(__m68k__) || defined(__hppa__) || defined(__arm__)
#          define NOPORTALIGN 1
#        endif
#      endif
#    endif
#  endif
#endif
#ifndef NOPORTALIGN
#  define NOPORTALIGN 0  /* Play it safe. It's needed by the large, compact and huge memory models on DOS. */
#endif

#endif  /* ifndef _AUTOC_H */
