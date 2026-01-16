#ifdef LIBCH
#  include "libc.h"
#else
#  ifndef MINIXHEAP
#    include <stdlib.h>  /* For malloc(...). */
#  endif
#  ifdef DEBUG_MINIXHEAP
#    include <sys/types.h>
#    include <unistd.h>
#  endif
#endif
#include "const.h"
#include "type.h"
#include "globvar.h"
#include "proto.h"

#ifndef MAXHEAP16  /* Configurable at compile time: -DMAXHEAP16=... */
#  ifdef MINIXHEAP
#    define MAXHEAP16 0xffff
#  else
#    define MAXHEAP16 0xac00  /* Used as MAXHEAP when sizeof(int) == 2. */
#  endif
#endif
#ifndef MAXHEAP32  /* Configurable at compile time: -DMAXHEAP32=...L */
#  define MAXHEAP32 0x20000L  /* Used as MAXHEAP when sizeof(int) > 2. */
#endif
#ifdef MAXHEAP  /* Configurable at compile time: -DMAXHEAP=... */
#  define MAXHEAPEXPR ((unsigned) (MAXHEAP))
#else
#  define MAXHEAPEXPR (sizeof(int) <= 2 ? (unsigned) (MAXHEAP16) : (unsigned) (MAXHEAP32))
#endif

#ifdef MINIXHEAP  /* Use brk(2) on Minix. It uses less code, and it wastes less data than malloc(3). */
extern char *brksize;  /* Defined by libc. */
extern char *brk();  /* Defined by libc. */

PRIVATE int trybrk(p) char *p; {
  /* brk(...) makes sure that a minimmum amount of stack space is still available above the heap. */
  return !brk(p) && brksize >= p;  /* brk(...) also changes brksize. */
}
#endif

PUBLIC void initheap()
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
  if (!a) as_abort("cannot allocate memory");  /* Minimum is 1 KiB. */
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
#else
  char *p;
  unsigned size;

  size = MAXHEAPEXPR;
  if (!(p = malloc(size))) {  /* If `size' bytes are not available, try smaller amounts. */
    while (size > 1024 && !(p = malloc((size >>= 1, size += size >> 1)))) {}
    if (!p && size > 1024 && !(p = malloc(size = 1024))) {
      as_abort("cannot allocate memory");  /* Minimum is 1 KiB. */
    }
  }
  heapend = (heapstart = heapptr = p) + size;
#endif
}
