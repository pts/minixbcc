/* typeconv.c - convert between little-endian char arrays and unsigneds */

/*
	u2c2(): 2 byte unsigned to 2 byte array
	u4c4(): 4 byte unsigned to 4 byte array
	u4cn(): 4 byte unsigned to n byte array
*/

#include "const.h"
#include "type.h"

#ifndef UALEMEM  /* Configurable from the command line. */  /* -DUALEMEM=1 means that the system is little-endian and supports unaligned memory access. If so, speed optimizations are possible. */
#  ifdef __BCC__
#    if __AS386_16__ || __AS386_32__
#      define UALEMEM 1
#    endif
#  else
#    ifdef __GNUC__  /* GCC, Clang, PCC or maybe others pretending to be GCC. */
#      if defined(__i386__) || defined(__x86_64__)  /* Play it safe, don't detect anything else. */
#        define UALEMEM 1
#      endif
#    else
#      ifdef __TINYC__  /* TinyCC. */
#        if defined(__i386__) || defined(__x86_64__)
#          define UALEMEM 1
#        endif
#      else
#        ifdef __WATCOMC__
#          if defined(_M_I86) || defined(__386__)
#            define UALEMEM 1
#          endif
#        else
#          ifdef __ACK__  /* Not defined in ACK 3.1 on Minix 1.5.10. __ACK__ defined in ACK ANSI C compiler 1.202 on Minix >=1.7.0, __i86 defined on Minix 2.0.4 (but not on Minix 1.7.0). */
#            if defined(__i86) || defined(__i386)
#              define UALEMEM 1
#            endif
#          endif
#        endif
#      endif
#    endif
#  endif
#endif
#ifndef UALEMEM
#  define UALEMEM 0  /* Play it safe. */
#endif

PUBLIC void u2c2 P2(REGISTER char *, buf, u2_pt, offset)
{
#if UALEMEM  /* Faster and shorter, but works only if the architecture supports it. */
    ((unsigned short*) buf)[0] = offset;
#else
    buf[1] = (char) ((unsigned) offset >> 8);
    buf[0] = (char) offset;
#endif
}

PUBLIC void u4c4 P2(REGISTER char *, buf, u4_pt, offset)
{
#if UALEMEM  /* Faster and shorter, but works only if the architecture supports it. */
    ((unsigned INT32T*) buf)[0] = offset;
#else
    u4cn(buf, offset, 4);
#endif
}

PUBLIC void u4cn P3(REGISTER char *, buf, u4_pt, offset, unsigned, count)
{
    switch (count)
    {
#if UALEMEM  /* Faster and shorter, but works only if the architecture supports it. */
    case 4:
	((unsigned INT32T*) buf)[0] = offset;
	break;
    case 2:
	((unsigned short*) buf)[0] = offset;
	break;
    case 1:
	buf[0] = (char) offset;
#else
    case 4:
	count = (unsigned) (offset >> 16);
	buf[3] = (char) (count >> 8);
	buf[2] = (char) count;
	/* Fallthrough. */
    case 2:
	buf[1] = (char) ((unsigned) offset >> 8);
	/* Fallthrough. */
    case 1:
	buf[0] = (char) offset;
#endif
    }
}
