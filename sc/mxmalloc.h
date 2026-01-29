/*
 * mxmalloc.c: sbrk(2), malloc(3), realloc(3) and free(3) for Minix i86 and i386, based on Minix 1.5.10 i86 libc /usr/src/lib/ansi/malloc.c
 */

#if __STDC__
#  define _PROTOTYPE(function, params)	function params
#  define _CONST	const
#else
#  define _PROTOTYPE(function, params)	function()
#  define _CONST
#endif /* _ANSI */

typedef unsigned size_t;

#ifdef SBRK
extern char *brksize;  /* Defined in assembly. */
#endif

extern char * _PROTOTYPE(brk, (char *addr));
extern char * _PROTOTYPE(sbrk, (int incr));
extern void * _PROTOTYPE(malloc, (unsigned size));
extern void * _PROTOTYPE(realloc, (void *oldfix, unsigned size));
extern void * _PROTOTYPE(memcpy, (void *_t, const void *_s, unsigned _length));

/* --- brk.c */

#ifdef SBRK
char *sbrk(incr)
int incr;
{
  char *newsize, *oldsize;

  oldsize = brksize;
  newsize = brksize + incr;
  if (incr > 0 && newsize < oldsize || incr < 0 && newsize > oldsize)
	return((char *) -1);
  if (brk(newsize) == 0)  /* This change brksize on success. */
	return(oldsize);
  else
	return((char *) -1);
}
#endif

/* --- malloc.c */

#define	ASSERT_MALLOC(b)		/* empty */

typedef int malloc_intptr_t;  /* This is architecture-dependent. */

#define BRKALIGN		1024
#define	PTRSIZE		sizeof(char *)
#define Align(x,a)	(((x) + (a - 1)) & ~(malloc_intptr_t)(a - 1))
#define NextSlot(p)	(* (char **) ((p) - PTRSIZE))
#define NextFree(p)	(* (char **) (p))

/* A short explanation of the data structure and algorithms.
 * An area returned by malloc() is called a slot. Each slot
 * contains the number of bytes requested, but preceeded by
 * an extra pointer to the next the slot in memory.
 * '_bottom' and '_top' point to the first/last slot.
 * More memory is asked for using brk() and appended to top.
 * The list of free slots is maintained to keep malloc() fast.
 * '_empty' points the the first free slot. Free slots are
 * linked together by a pointer at the start of the
 * user visable part, so just after the next-slot pointer.
 * Free slots are merged together by free().
 */

static char *_bottom, *_top, *_empty;

static _PROTOTYPE(int grow, (unsigned len));
_PROTOTYPE(void free, (void *pfix));

static int grow(len)
unsigned len;
{
  register char *p;

  ASSERT_MALLOC(NextSlot(_top) == 0);
  p = (char *) Align((malloc_intptr_t) _top + len, BRKALIGN);
  if (p < _top || brk(p) != 0) return(0);
  NextSlot(_top) = p;
  NextSlot(p) = 0;
  free(_top);
  _top = p;
  return(1);
}

void *malloc(size)
unsigned size;
{
  register char *prev, *p, *next, *new;
  register unsigned len, ntries;

  if (size == 0) size = PTRSIZE;/* avoid slots less that 2*PTRSIZE */
  for (ntries = 0; ntries < 2; ntries++) {
	if ((len = Align(size, PTRSIZE) + PTRSIZE) < 2 * PTRSIZE)
		return(0);	/* overflow */
	if (_bottom == 0) {
		if ((p = sbrk(2 * PTRSIZE)) == (char *) -1) return(0);
		p = (char *) Align((malloc_intptr_t) p, PTRSIZE);
		ASSERT_MALLOC(p + PTRSIZE > p);	/* sbrk amount stops
						 * overflow */
		p += PTRSIZE;
		_top = _bottom = p;
		NextSlot(p) = 0;
	}
	for (prev = 0, p = _empty; p != 0; prev = p, p = NextFree(p)) {
		next = NextSlot(p);
		new = p + len;	/* easily overflows! */
		if (new > next || new <= p) continue;	/* too small */
		if (new + PTRSIZE < next) {	/* too big, so split */
			/* + PTRSIZE avoids tiny slots on free list */
			ASSERT_MALLOC(new + PTRSIZE > new);	/* space above next */
			NextSlot(new) = next;
			NextSlot(p) = new;
			NextFree(new) = NextFree(p);
			NextFree(p) = new;
		}
		if (prev)
			NextFree(prev) = NextFree(p);
		else
			_empty = NextFree(p);
		return((void *)p);
	}
	if (grow(len) == 0) break;
  }
  ASSERT_MALLOC(ntries != 2);
  return((void *)0);
}

void *realloc(oldfix, size)
void *oldfix;
unsigned size;
{
  register char *prev, *p, *next, *new;
  register unsigned len, n;
  char *old = (char *) oldfix;


  if (size > ~(unsigned) (2 * PTRSIZE) + 1) return(0);
  len = Align(size, PTRSIZE) + PTRSIZE;
  next = NextSlot(old);
  n = (int) (next - old);	/* old length */
  /* Extend old if there is any free space just behind it */
  for (prev = 0, p = _empty; p != 0; prev = p, p = NextFree(p)) {
	if (p > next) break;
	if (p == next) {	/* 'next' is a free slot: merge */
		NextSlot(old) = NextSlot(p);
		if (prev)
			NextFree(prev) = NextFree(p);
		else
			_empty = NextFree(p);
		next = NextSlot(old);
		break;
	}
  }
  new = old + len;		/* easily overflows! */
  /* Can we use the old, possibly extended slot? */
  if (new <= next && new >= old) {	/* it does fit */
	if (new + PTRSIZE < next) {	/* too big, so split */
		/* + PTRSIZE avoids tiny slots on free list */
		ASSERT_MALLOC(new + PTRSIZE > new);
		NextSlot(new) = next;
		NextSlot(old) = new;
		free(new);
	}
	return((void *)old);
  }
  if ((new = (char *)malloc(size)) == (char*) 0)  /* it didn't fit */
	return((void *) 0);
  memcpy(new, old, (size_t)n);		/* n < size */
  free(old);
  return((void *)new);
}

void free(pfix)
void *pfix;
{
  register char *prev, *next;
  char *p = (char *) pfix;

  ASSERT_MALLOC(NextSlot(p) > p);
  for (prev = 0, next = _empty; next != 0; prev = next, next = NextFree(next))
	if (p < next) break;
  NextFree(p) = next;
  if (prev)
	NextFree(prev) = p;
  else
	_empty = p;
  if (next) {
	ASSERT_MALLOC(NextSlot(p) <= next);
	if (NextSlot(p) == next) {	/* merge p and next */
		NextSlot(p) = NextSlot(next);
		NextFree(p) = NextFree(next);
	}
  }
  if (prev) {
	ASSERT_MALLOC(NextSlot(prev) <= p);
	if (NextSlot(prev) == p) {	/* merge prev and p */
		NextSlot(prev) = NextSlot(p);
		NextFree(prev) = NextFree(p);
	}
  }
}
