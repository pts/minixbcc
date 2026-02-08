#ifndef MINIXHEAP  /* Configurable from the command line. */
#  ifndef NOMINIXHEAP
#    ifdef __MINIX__
#      define MINIXHEAP 1
#    else
#      ifdef __MINIX  /* The ACK ANSI C compiler 1.202 in Minix >=1.7.0 defines it. */
#        define MINIXHEAP 1
#      else
#        ifdef __ELKS__
#          define MINIXHEAP 1
#        endif
#      endif
#    endif
#  endif
#endif

#ifdef LIBCH
#  include "libc.h"
#else
#  ifdef MINIXHEAP
#    ifndef _MINIX  /* Make <unistd.h> declare brk(...) and sbrk(...) for ACK ANSI C compiler 1.202. */
#      define _MINIX 1
#    endif
#    include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#    include <unistd.h>  /* For brk(...) and sbrk(...). */
#  else
#    include <stdlib.h>  /* For malloc(...). */
#  endif
#endif
#include "const.h"
#include "type.h"
#include "globvar.h"


#ifndef MAXHEAP16  /* Configurable at compile time: -DMAXHEAP16=... */
#  ifdef MINIXHEAP
#    define MAXHEAP16 0xffff
#  else
#    define MAXHEAP16 0xe000  /* Used as MAXHEAP when sizeof(char *) == 2. */
#  endif
#endif
#ifndef MAXHEAP32  /* Configurable at compile time: -DMAXHEAP32=...L */
#  define MAXHEAP32 0x30000L  /* Used as MAXHEAP when sizeof(char *) > 2. */
#endif
#ifdef MAXHEAP  /* Configurable at compile time: -DMAXHEAP=... */
#  define MAXHEAPEXPR ((unsigned) (MAXHEAP))
#else
  /* The `& ~(unsigned) 0' pacifies the ACK 3.1 warning on Minix 1.5.10: overflow in unsigned constant expression */
#  define MAXHEAPEXPR (sizeof(char *) <= 2 ? (unsigned) (MAXHEAP16) : (unsigned) ((MAXHEAP32) & (unsigned) ~0))
#endif

#ifdef MINIXHEAP  /* Use brk(2) on Minix. It uses less code, and it wastes less data than malloc(3). */
#ifdef BRKSIZE
  extern char *brksize;  /* Defined by libc. */
#else
#  define brksize sbrk(0)  /* Minix 2.0.4 libc doesn't have the global variable brksize (but it has _brksize), so we use sbrk(0) instead, which works everywhere. */
#endif

PRIVATE int trybrk P((char *p));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
PRIVATE int trybrk P1(char *, p)
{
  /* brk(...) makes sure that a minimmum amount of stack space is still available above the heap. */
  return !brk(p) && brksize >= p;  /* brk(...) also changes brksize. */
}
#endif

PUBLIC void initheap P0()
{
#ifdef MINIXHEAP
  unsigned a, b, m;

  if (heapstart) return;  /* Already called. */

  /* Use binary search to find the maximum heap size available, and allocate it. */
  heapptr = brksize;
#ifdef DEBUG_MINIXHEAP
  (void)!write(2, "I", 1);
#endif
  for (a = 0, b = 1024, m = MAXHEAPEXPR; heapptr + b > heapptr && trybrk(heapptr + b); ) {
    a = b;
    if (b >= m) break;
    if ((b <<= 1) > m) b = m;
#ifdef DEBUG_MINIXHEAP
    (void)!write(2, ",", 1);
#endif
  }
  if (!b) --b;  /* Decrease on power-of-2 overflow to 0. */
#ifdef DEBUG_MINIXHEAP
  (void)!write(2, "_", 1);
#endif
  if (!a) outofmemory();  /* Minimum is 1 KiB. */
  for (;;) {
    /* Now `a' bytes are available, and `b' bytes are not available, and `a < b'. */
    if ((m = a + ((b - a) >> 1)) == a) break;
#ifdef DEBUG_MINIXHEAP
    (void)!write(2, "?", 1);
#endif
    if (heapptr + m > heapptr && trybrk(heapptr + m)) {
#ifdef DEBUG_MINIXHEAP
      (void)!write(2, "A", 1);
#endif
      if (brksize > heapptr + m) {
        if ((a = brksize - heapptr) >= b) break;
      } else {
        a = m;
      }
    } else {
#ifdef DEBUG_MINIXHEAP
      (void)!write(2, "B", 1);
#endif
      b = m;
    }
  }
  heapend = (heapstart = heapptr) + a;
#ifdef DEBUG_MINIXHEAP
  (void)!write(2, ".\n", 2);
#endif
#else  /* #ifdef MINIXHEAP */
  char *p;
  unsigned size;

  size = MAXHEAPEXPR;
  if (!(p = (char *) malloc(size))) {  /* If `size' bytes are not available, try smaller amounts. */
    while (size > 1024 && !(p = (char *) malloc((size >>= 1, size += size >> 1)))) {}
    if (!p && size > 1024 && !(p = (char *) malloc(size = 1024))) {
      outofmemory();  /* Minimum is 1 KiB. */
    }
  }
  heapend = (heapstart = heapptr = p) + size;
#endif  /* #else #ifdef MINIXHEAP */
}
