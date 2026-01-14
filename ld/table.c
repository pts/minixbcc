/* table.c - table-handler module for linker */

#include "const.h"
#include "align.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#define GOLDEN 157		/* GOLDEN/HASHTABSIZE approx golden ratio */
#define HASHTABSIZE 256

PUBLIC char *heapstart;		/* start of heap (catchall table) heapstart <= heapptr <= heapend */
PUBLIC char *heapend;		/* end of free space in heap */
PUBLIC char *heapptr;		/* next free space in heap */

PRIVATE struct symstruct *hashtab[HASHTABSIZE];	/* hash table */

FORWARD struct symstruct **gethashptr P((char *name));

/* initialise symbol table */

PUBLIC void syminit()
{
    unsigned i;

    initheap();  /* Initializes heapstart, heapptr and heapend. */
    for (i = 0; i < HASHTABSIZE; i++)
	hashtab[i] = NULL;
}

/* add named symbol to end of table - initialise only name and next fields */
/* caller must not duplicate names of externals for findsym() to work */

PUBLIC struct symstruct *addsym(name)
char *name;
{
    struct symstruct **hashptr;
    struct symstruct *oldsymptr;
    struct symstruct *symptr;

    oldsymptr = NULL;  /* Pacify GCC -Wmaybe-uninitialized warning below. */
    hashptr = gethashptr(name);
    symptr = *hashptr;
    while (symptr != NULL)
    {
	oldsymptr = symptr;
	symptr = symptr->next;
    }
    align(heapptr);
    symptr = (struct symstruct *) heapptr;
    if ((heapptr = symptr->name + (strlen(name) + 1)) > heapend)
	outofmemory();
    symptr->modptr = NULL;
    symptr->next = NULL;
    if (name != symptr->name)
	strcpy(symptr->name, name);	/* should't happen */
    if (*hashptr == NULL)
	*hashptr = symptr;
    else if (oldsymptr)
	oldsymptr->next = symptr;
    return symptr;
}

/* lookup named symbol */

PUBLIC struct symstruct *findsym(name)
char *name;
{
    struct symstruct *symptr;

    symptr = *gethashptr(name);
    while (symptr != NULL && (!(symptr->flags & (E_MASK | I_MASK)) ||
			      strcmp(symptr->name, name) != 0))
	symptr = symptr->next;
    return symptr;
}

/* convert name to a hash table ptr */

PRIVATE struct symstruct **gethashptr(name)
register char *name;
{
    register unsigned hashval;

    hashval = 0;
    while (*name)
	hashval = hashval * 2 + *name++;
    return hashtab + ((hashval * GOLDEN) & (HASHTABSIZE - 1));

/*

#asm

GOLDEN	EQU	157
HASHTABSIZE	EQU	256

	CLRB		can build value here since HASHTABSIZE <= 256
	LDA	,X
	BEQ	HASHVAL.EXIT
HASHVAL.LOOP
	ADDB	,X+
	LSLB
	LDA	,X
	BNE	HASHVAL.LOOP
	RORB
	LDA	#GOLDEN
	MUL
HASHVAL.EXIT
HASHVAL.EXIT
	LDX	#_hashtab
	ABX			discard	A - same as taking mod HASHTABSIZE
	ABX
#endasm

*/

}

/* move symbol descriptor entries to top of table (no error checking) */

PUBLIC char *moveup(nbytes)
unsigned nbytes;
{
    register char *source;
    register char *target;

    source = heapptr;
    target = heapend;
    while (nbytes--)
	*--target = *--source;
    heapptr = source;
    return heapend = target;
}

/* allocate from our heap */

PUBLIC char *heapalloc(nbytes)
unsigned nbytes;
{
    char *allocptr;

    align(heapptr);
    allocptr = heapptr;
    if ((heapptr += nbytes) > heapend)
	outofmemory();
    return allocptr;
}

/* read string from file into table at offset suitable for next symbol */

PUBLIC char *readstring()
{
    int c;
    char *s;
    char *start;

    align(heapptr);
    start = s = ((struct symstruct *) heapptr)->name;
    while (TRUE)
    {
	if (s >= heapend)
	    outofmemory();
	if ((c = readchar()) < 0)
	    prematureeof();
	if ((*s++ = c) == 0)
	    return start;
    }
    /* NOTREACHED */
}

/* release from top of table */

PUBLIC void release(cptr)
char *cptr;
{
    heapend = cptr;
}

/* allocate space for string */

PUBLIC char *stralloc(s)
char *s;
{
    return strcpy(heapalloc((unsigned) strlen(s) + 1), s);
}
