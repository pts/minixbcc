/* type.h - types for linker */

#ifndef _AUTOC_H
#  include "autoc.h"  /* For INT32T. */
#endif

typedef unsigned short u2_t;
typedef unsigned u2_pt;
typedef unsigned INT32T u4_t;
typedef unsigned INT32T u4_pt;
typedef char assert_sizeof_u4_pt[sizeof(u4_pt) >= sizeof(int) ? 1 : -1];  /* Make sure that u4_t defined above doesn't promote to anything larger than u4_pt. */

typedef unsigned char bool_t;
typedef unsigned bool_pt;	/* !! change to int for ANSI C */

typedef unsigned INT32T offset_t;

#ifdef OBJ_H			/* obj.h is included */

typedef unsigned flags_t;	/* unsigned makes shifts logical */

struct entrylist		/* list of entry symbols */
{
    struct entrylist *elnext;	/* next on list */
    struct symstruct *elsymptr;	/* entry on list */
};

struct modstruct		/* module table entry format */
{
    _CONST char *filename;	/* file containing this module */
    _CONST char *archentry;	/* name of component file for archives */
    _CONST char *modname;	/* name of module */
    offset_t textoffset;	/* offset to module text in file */
    char class_;		/* class of module */
    char loadflag;		/* set if module to be loaded */
    char segmaxsize[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for seg max size */
				/* 00 = 1, 01 = 2, 10 = 3, 11 = 4 */
    char segsizedesc[NSEG / 4];	/* |SF|SE|..|S0|, 2 bits for #bytes for size */
				/* 00 = 0, 01 = 1, 10 = 2, 11 = 4 */
    struct symstruct **symparray;	/* ^array of ptrs to referenced syms */
    struct modstruct *modnext;	/* next module in order of initial reading */
#ifdef DEBUG_SIZE
    offset_t modcomsz;		/* total size of common symbols (including alignment) defined in this module */
#else
#  ifdef DEBUG_SIZE_NOPAD
    offset_t modcomsz;  /* See above. */
#  endif
#endif
    char segsize[1];		/* up to 64 size bytes begin here */
};				/* careful with sizeof( struct modstruct )!! */

struct redlist			/* list of redefined (exported) symbols */
{
    struct redlist *rlnext;	/* next on list */
    struct symstruct *rlsymptr;	/* to symbol with same name, flags */
    struct modstruct *rlmodptr;	/* module for this redefinition */
    offset_t rlvalue;		/* value for this redefinition */
};

struct symstruct		/* symbol table entry format */
{
    struct modstruct *modptr;	/* module where symbol is defined */
    offset_t value;		/* value of symbol */
    flags_t flags;		/* see below (unsigned makes shifts logical) */
    struct symstruct *next;	/* next symbol with same hash value */
    char name[1];		/* name is any string beginning here */
};				/* don't use sizeof( struct symstruct )!! */

#endif				/* obj.h is included */

/* prototypes */

/* dump.c */
void dumpmods P((void));
void dumpsyms P((void));

/* heap.c */
void initheap P((void));

/* io.c */
void ioinit P((_CONST char *progname));
void closein P((void));
void closeout P((void));
void executable P((void));
void flusherr P((void));
void openin P((_CONST char *filename));
void openout P((_CONST char *filename));
void putstr P((_CONST char *message));
void puthexdig P((unsigned num));
#ifdef OBJ_H
void put08x P((offset_t num));
void put08lx P((offset_t num));
#endif
void putbstr P((unsigned width, _CONST char *str));
void putbyte P((int ch));
int readchar P((void));
void readin P((char *buf, unsigned count));
bool_pt readineofok P((char *buf, unsigned count));
void seekin P((unsigned INT32T offset));
void seekout P((unsigned INT32T offset));
void writechar P((int c));
void writeout P((_CONST char *buf, unsigned count));
void refer P((void));
void errexit P((_CONST char *message));
void fatalerror P((_CONST char *message));
void inputerror P((_CONST char *message));
void input1error P((_CONST char *message));
void outofmemory P((void));
void prematureeof P((void));
void redefined P((_CONST char *name, _CONST char *message, _CONST char *archentry,
		  _CONST char *deffilename, _CONST char *defarchentry));
void reserved P((_CONST char *name));
#ifdef OBJ_H
void size_error P((unsigned seg, offset_t count, offset_t size));
#endif
void undefined P((_CONST char *name));
void usage P((void));

/* ld.c */
int main P((int argc, char **argv));

/* readobj.c */
void objinit P((void));
void readsyms P((_CONST char *filename));
bool_pt parse_nonneg_lenient P((_CONST char *s, unsigned base, offset_t *output));
#ifdef OBJ_H
void entrysym P((struct symstruct *symptr));
offset_t readconvsize P((unsigned countindex));
offset_t readsize P((unsigned count));
unsigned segsizecount P((unsigned seg, struct modstruct *modptr));
#endif

/* table.c */
void syminit P((void));
struct symstruct *addsym P((_CONST char *name));
struct symstruct *findsym P((_CONST char *name));
char *moveup P((unsigned nbytes));
char *heapalloc P((unsigned nbytes));
void ourfree P((char *cptr));
char *readstring P((void));
void release P((char *cptr));
char *stralloc P((_CONST char *s));

/* typeconvert.c */
u2_pt c2u2 P((_CONST char *buf));
u4_pt c4u4 P((_CONST char *buf));
u2_pt cnu2 P((_CONST char *buf, unsigned count));
u4_pt cnu4 P((_CONST char *buf, unsigned count));
void u2c2 P((char *buf, u2_pt offset));
void u4c4 P((char *buf, u4_pt offset));
void u2cn P((char *buf, u2_pt offset, unsigned count));
void u4cn P((char *buf, u4_pt offset, unsigned count));
bool_pt typeconv_init P((void));

/* writebin.c */
void writebin P((_CONST char *outfilename, bool_pt argsepid, bool_pt argbits32,
		 bool_pt argstripflag, bool_pt arguzp));
void linksyms P((bool_pt argreloc_output));
