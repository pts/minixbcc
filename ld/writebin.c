/* writebin.c - write binary file for linker */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <string.h>
#endif
#ifndef _AUTOC_H
#  include "autoc.h"  /* For INT32T. */
#endif
#include "const.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#ifdef DEBUG_SIZE_NOPAD
#  ifndef DEBUG_SIZE
#    define DEBUG_SIZE
#  endif
#endif
#ifdef DEBUG_SIZE
#  include <stdio.h>
#  ifdef __WATCOMC__  /* OpenWatcom v2 or earlier. */
#    ifdef _WCDATA  /* OpenWatcom v2 libc. */
#      ifdef __LINUX__  /* owcc -blinux */
        FILE *my_stderr_ptr;  /* Workaround for `sh build.sh owcc', without the `-I"$WATCOM"/lh' or `export INCLUDE=$WATCOM/lh', and using (by default) -I"$WATCOM"/h instead of the correct -I"$WATCOM"/lh on Linux. */
        FILE *my_stderr_get(void) { if (!my_stderr_ptr) my_stderr_ptr = fdopen(2, "w"); return my_stderr_ptr; }
#        undef  stderr  /* The OpenWatcom v2 libc defines it. */
#        define stderr my_stderr_get()
#      endif
#    endif
#  endif
#endif

/* --- Minix a.out executable file format support */

#if 0  /* Not used to avoid byte order and alignment issues, a char[A_MINHDR] is used instead. */
struct exec {  /* Minix a.out executable header. */
	uint8_t		a_magic[2];	/* magic number: {1, 3} for little endian */
	uint8_t		a_flags;	/* flags, see below */
	uint8_t		a_cpu;		/* cpu id : A_I8085, A_I80386 etc. */
	uint8_t		a_hdrlen;	/* length of header */
	uint8_t		a_unused;	/* reserved for future use */
	uint16_t	a_version;	/* version stamp, not used */
	int32_t		a_text;		/* size of text segement in bytes */
	int32_t		a_data;		/* size of data segment in bytes */
	int32_t		a_bss;		/* size of bss segment in bytes */
	int32_t		a_entry;	/* entry point */
	int32_t		a_total;	/* total memory allocated */
	int32_t		a_syms;		/* size of symbol table */
};
typedef char assert_sizeof_exec[sizeof(struct exec) == 32 ? 1 : -1];
#endif

#define OFFSETOF_a_text 8
#define OFFSETOF_a_data 0xc
#define OFFSETOF_a_bss  0x10
#define OFFSETOF_a_entry 0x14
#define OFFSETOF_a_total 0x18
#define OFFSETOF_a_syms 0x1c
#define A_MINHDR	0x20	/* Byte size of the short form of struct exec. */

#define A_I8086		0x04	/* intel i8086/8088 */
#define A_I80386	0x10	/* intel i80386 */

/* flags: */
#define A_UZP		1	/* unallocated zero page */
#define A_EXEC		0x10	/* executable */
#define A_SEP		0x20	/* separate I/D */

#if 0  /* Not used to avoid byte order and alignment issues, a char[A_LISTHDR] is used instead. */
struct nlist {  /* Minix a.out symbol table entry */  /* !! Don't use it. */
	char	 	n_name[8];	/* symbol name */
	int32_t	 	n_value;	/* value: signed, little-endian 32-bit integer */
	uint8_t		n_sclass;	/* storage class: unsigned byte */
	uint8_t		n_numaux;	/* number of auxiliary entries, not used, byte */
	uint16_t	n_type;	/* language base and derived type. not used, unsigned, little-endian 16-bit integer */
};
typedef char assert_sizeof_nlist[sizeof(struct nlist) == 16 ? 1 : -1];
#endif

#define OFFSETOF_n_name 0
#define OFFSETOF_n_value 8
#define OFFSETOF_n_sclass 0xc
#define OFFSETOF_n_unused 0xd
#define A_LISTHDR	0x10  /* Byte size of the Minix a.out symbol table entry .*/

/* low bits of storage class (section) */
#define	N_SECT		  07	/* section mask */
#define N_UNDF		  00	/* undefined */
#define N_ABS		  01	/* absolute */
#define N_TEXT		  02	/* text */
#define N_DATA		  03	/* data */
#define	N_BSS		  04	/* bss */
#define N_COMM		  05	/* (common), unused in executables */

/* high bits of storage class */
#define N_CLASS		0370	/* storage class mask */
#define C_NULL
#define C_EXT		0020	/* external symbol */
#define C_STAT		0030	/* static */

/* --- */

#define page_size() 4096

#define DPSEG 2

#define CM_MASK 0xC0
#define MODIFY_MASK 0x3F
#define S_MASK 0x04
#define OF_MASK 0x03

#define CM_SPECIAL 0
#define CM_ABSOLUTE 0x40
#define CM_OFFSET_RELOC 0x80
#define CM_SYMBOL_RELOC 0xC0

#define CM_EOT 0
#define CM_BYTE_SIZE 1
#define CM_WORD_SIZE 2
#define CM_LONG_SIZE 3
#define CM_1_SKIP 17
#define CM_2_SKIP 18
#define CM_4_SKIP 19
#define CM_0_SEG 32

#define ABS_TEXT_MAX 64

PRIVATE bool_t bits32;		/* nonzero for 32-bit executable */
PRIVATE offset_t combase[NSEG];	/* bases of common parts of segments */
PRIVATE offset_t comsz[NSEG];	/* sizes of common parts of segments */
PRIVATE unsigned curseg;	/* current segment, 0 to $F */
PRIVATE offset_t edataoffset;	/* end of data */
PRIVATE offset_t endoffset;	/* end of bss */
PRIVATE offset_t etextoffset;	/* end of text */
PRIVATE offset_t etextpadoff;	/* end of padded text */
PRIVATE unsigned nsym;		/* number of symbols written */
PRIVATE unsigned relocsize;	/* current relocation size 1, 2 or 4 */
PRIVATE offset_t segadj[NSEG];	/* adjusts (file offset - seg offset) */
				/* depends on zero init */
PRIVATE offset_t segbase[NSEG];	/* bases of data parts of segments */
PRIVATE char segboundary[9] = "__seg0DH";
				/* name of seg boundary __seg0DL to __segfCH */
PRIVATE offset_t segpos[NSEG];	/* segment positions for current module */
PRIVATE offset_t segsz[NSEG];	/* sizes of data parts of segments */
				/* depends on zero init */
PRIVATE bool_t sepid;		/* nonzero for separate I & D */
PRIVATE bool_t stripflag;	/* nonzero to strip symbols */
PRIVATE offset_t spos;		/* position in current seg */
PRIVATE bool_t uzp;		/* nonzero for unmapped zero page */

PUBLIC offset_t btextoffset;  /* text base address; default: 0 */
PUBLIC offset_t bdataoffset;  /* data base address: will be computed automatically */
PUBLIC offset_t dynam_size;   /* desired dynamic memory size (including heap, stack, argv and environ); the default of 0 means automatic */

FORWARD void linkmod P((struct modstruct *modptr));
FORWARD void linkrefs P((struct modstruct *modptr));
FORWARD void padmod P((struct modstruct *modptr));
FORWARD void setsym P((_CONST char *name, offset_t value));
FORWARD void symres P((_CONST char *name));
FORWARD void setseg P((unsigned newseg));
FORWARD void skip P((unsigned countsize));
FORWARD void writeheader P((void));
FORWARD void writenulls P((offset_t count));

/* link all symbols connected to entry symbols */

PUBLIC void linksyms P1(bool_pt, argreloc_output)
{
    char needlink;
    struct entrylist *elptr;
    struct modstruct *modptr;
    struct symstruct *symptr;

    (void)argreloc_output;  /* !! Size optimization: remove this argument. */
    if ((symptr = findsym("_main")) != (struct symstruct*) 0)
	entrysym(symptr);
    do
    {
	if ((elptr = entryfirst) == (struct entrylist*) 0)
	    fatalerror("no start symbol");
	for (modptr = modfirst; modptr != (struct modstruct*) 0; modptr = modptr->modnext)
	    modptr->loadflag = FALSE;
	for (; elptr != (struct entrylist*) 0; elptr = elptr->elnext)
	    linkrefs(elptr->elsymptr->modptr);
	if ((symptr = findsym("start")) != (struct symstruct*) 0 ||
	    (symptr = findsym("crtso")) != (struct symstruct*) 0)
	    linkrefs(symptr->modptr);
	needlink = FALSE;
	{
	    struct redlist *prlptr;
	    struct redlist *rlptr;

	    prlptr = (struct redlist*) 0;  /* Pacify GCC -Wmaybe-uninitialized warning below. */
	    for (rlptr = redfirst; rlptr != (struct redlist*) 0;
		 rlptr = (prlptr = rlptr)->rlnext)
		if (rlptr->rlmodptr->loadflag &&
		    rlptr->rlmodptr->class_ > rlptr->rlsymptr->modptr->class_)
		{
		    rlptr->rlsymptr->modptr = rlptr->rlmodptr;
		    rlptr->rlsymptr->value = rlptr->rlvalue;
		    if (rlptr == redfirst)
			redfirst = rlptr->rlnext;
		    else
			prlptr->rlnext = rlptr->rlnext;
		    needlink = TRUE;
		}
	}
    }
    while (needlink);
}

PRIVATE void checksize P((void));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
PRIVATE void checksize P0() {
    if (!bits32) {
	if (etextoffset > (offset_t) 0xff00L) fatalerror("a_text too large");  /* This is the actual limit for a_text in Minix 1.5.10 i86. */
	if (endoffset > (offset_t) 0xffc0L) fatalerror("a_data+a_bss too large");  /* 0x20 bytes for the stack. */
    }
}

PRIVATE void namecpy P((char *p, char *pend, _CONST char *src));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
PRIVATE void namecpy P3(char *, p, char *, pend, _CONST char *, src)
{
	REGISTER char *q;

	for (q = p; q != pend && (*q++ = *src++) != '\0'; ) {}
	for (; q != pend; *q++ = '\0') {}
}

#ifdef ACKFIX  /* Helper function used below. */
PRIVATE offset_t *addcomseg P((struct symstruct *symptr));  /* Declare to pacify the ACK ANSI C compiler 1.202 warning: old-fashioned function declaration. */
PRIVATE offset_t *addcomseg P1(struct symstruct *, symptr)
{
    REGISTER offset_t *comszseg;

    symptr->value = *(comszseg = &comsz[symptr->flags & SEGM_MASK]);
    return comszseg;
}
#endif

/* write binary file */

PUBLIC void writebin P5(_CONST char *, outfilename, bool_pt, argsepid, bool_pt, argbits32, bool_pt, argstripflag, bool_pt, arguzp)
{
    char buf4[4];
    char *cptr;
    char extsym[A_LISTHDR];  /* !! Don't use this struct, to avoid alignment issues during serialization. */
    flags_t flags;
    struct modstruct *modptr;
    unsigned seg;
    unsigned sizecount;
    offset_t tempoffset;

    sepid = argsepid;
    bits32 = argbits32;
    stripflag = argstripflag;
    uzp = arguzp;
    if (uzp)
    {
	if (btextoffset == 0)
	    btextoffset = page_size();
	if (bdataoffset == 0 && sepid)
	    bdataoffset = page_size();
    }

    /* reserve special symbols use curseg to pass parameter to symres() */
    for (curseg = 0; curseg < NSEG; ++curseg)
    {
	segboundary[5] = hexdigit[curseg];	/* to __segX?H */
	segboundary[6] = 'D';
	symres(segboundary);	/* __segXDH */
	segboundary[7] = 'L';
	symres(segboundary);	/* __segXDL */
	segboundary[6] = 'C';
	symres(segboundary);	/* __segXCL */
	segboundary[7] = 'H';
	symres(segboundary);	/* __segXCH */
    }
    curseg = 3;			/* data seg, s.b. variable */
    symres("_edata");
    symres("_end");
    curseg = 0;			/* text seg, s.b. variable */
    symres("_etext");

    /* calculate segment and common sizes (sum over loaded modules) */
    /* use zero init of segsz[] */
    /* also relocate symbols relative to starts of their segments */
    for (modptr = modfirst; modptr != (struct modstruct*) 0; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    REGISTER struct symstruct **symparray;
	    REGISTER struct symstruct *symptr;

#ifdef DEBUG_SIZE
	    modptr->modcomsz = 0;
#endif
	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != (struct symstruct*) 0; ++symparray)
		if (symptr->modptr == modptr && !(symptr->flags & A_MASK))
		{
		    if (!(symptr->flags & (I_MASK | SA_MASK)))
		    {
			/* relocate by offset of module in segment later */
			/* relocate by offset of segment in memory special */
			/* symbols get relocated improperly */
			symptr->value += segsz[symptr->flags & SEGM_MASK];
		    }
		    else if (symptr->value == 0)
		    {
			    undefined(symptr->name);
		    }
		    else
		    {
			{  /* C_MASK: common symbol. */
			    tempoffset = ldroundup(symptr->value, 4, offset_t);
#ifdef DEBUG_SIZE
			    modptr->modcomsz += tempoffset;
#endif
			    /* temp kludge quad alignment for 386 */
#ifdef ACKFIX  /* For Minix 1.5.10 i86 ACK 3.1 C compiler in optimized mode (cc -O). */
			    *addcomseg(symptr) += tempoffset;    /* It works with any C compiler. */
#else  /* The Minix 1.5.10 i86 ACK 3.1 C compiler is buggy: `/usr/lib/cg -p4' crash-fails for this with: Error: Bombed out of codegen */
			    symptr->value = comsz[seg = symptr->flags & SEGM_MASK];
			    comsz[seg] += tempoffset;
#endif
			}
			if (!(symptr->flags & SA_MASK))
			    symptr->flags |= C_MASK;
		    }
		}
	    for (seg = 0, cptr = modptr->segsize; seg < NSEG; ++seg)
	    {
		segsz[seg] += cnu4(cptr, sizecount = segsizecount((unsigned) seg, modptr));

		/* adjust sizes to even to get quad boundaries */
		/* this should be specifiable dynamically */
		segsz[seg] = ldroundup(segsz[seg], 4, offset_t);
		comsz[seg] = ldroundup(comsz[seg], 4, offset_t);
		cptr += sizecount;
	    }
	}

    /* calculate seg positions now their sizes are known */
    /* temp use fixed order 0D 0C 1D 1C 2D 2C ... */
    /* assume seg 0 is text and rest are data */
    segpos[0] = segbase[0] = spos = btextoffset;
    combase[0] = segbase[0] + segsz[0];
    segadj[1] = segadj[0] = -btextoffset;
    etextpadoff = etextoffset = combase[0] + comsz[0];
    if (sepid)
    {
	etextpadoff = ldroundup(etextoffset, 0x10, offset_t);
	segadj[1] += etextpadoff - bdataoffset;
    }
    else if (bdataoffset == 0)
	bdataoffset = etextpadoff;
    segpos[1] = segbase[1] = edataoffset = bdataoffset;
    combase[1] = segbase[1] + segsz[1];
    for (seg = 2; seg < NSEG; ++seg)
    {
	segpos[seg] = segbase[seg] = combase[seg - 1] + comsz[seg - 1];
	if (seg == DPSEG)
	{
	    /* temporarily have fixed DP seg */
	    /* adjust if nec so it only spans 1 page */
	    tempoffset = segsz[seg] + comsz[seg];
	    if (tempoffset > 0x100)
		fatalerror("direct page segment too large");
	    if ((((segbase[seg] + tempoffset) ^ segbase[seg]) & ~0xFF) != 0)
		segpos[seg] = segbase[seg] = (segbase[seg] + 0xFF) & ~0xFF;
	}
	combase[seg] = segbase[seg] + segsz[seg];
	segadj[seg] = segadj[seg - 1];
    }

    /* relocate symbols by offsets of segments in memory */
    for (modptr = modfirst; modptr != (struct modstruct*) 0; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    REGISTER struct symstruct **symparray;
	    REGISTER struct symstruct *symptr;

	    for (symparray = modptr->symparray;
		 (symptr = *symparray) != (struct symstruct*) 0; ++symparray)
		if (symptr->modptr == modptr && !(symptr->flags & A_MASK))
		{
		    symptr->value += ((symptr->flags & (C_MASK | SA_MASK)) ? combase : segbase)[symptr->flags & SEGM_MASK];
		}
	}

    /* adjust special symbols */
    for (seg = 0; seg < NSEG; ++seg)
    {
	if (segsz[seg] != 0)
	    /* only count data of nonzero length */
	    edataoffset = segbase[seg] + segsz[seg];
	segboundary[5] = hexdigit[seg];		/* to __segX?H */
	segboundary[6] = 'D';
	setsym(segboundary, (tempoffset = segbase[seg]) + segsz[seg]);
						/* __segXDH */
	segboundary[7] = 'L';
	setsym(segboundary, tempoffset);	/* __segXDL */
	segboundary[6] = 'C';
	setsym(segboundary, tempoffset = combase[seg]);
						/* __segXCL */
	segboundary[7] = 'H';
	setsym(segboundary, tempoffset + comsz[seg]);
						/* __segXCH */
    }
    endoffset = combase[NSEG - 1] + comsz[NSEG - 1];
#ifdef DEBUG_SIZE
#  ifdef DEBUG_SIZE_NOPAD
#    define ETEXTMAYBEPADOFF etextoffset
#  else
#    define ETEXTMAYBEPADOFF etextpadoff
#  endif
    /* These values are correct for both values of sepid, and for uzp == 0. They may be correct with uzp == 1 as well. */
    fprintf(stderr, "info: total executable size: a_text=%lu a_data=%lu a_bss=%lu a_data+a_bss=%lu\n", (unsigned long) (ETEXTMAYBEPADOFF - btextoffset), (unsigned long) (edataoffset - bdataoffset), (unsigned long) (endoffset - edataoffset), (unsigned long) (endoffset));
    fflush(stderr);
#else  /* For DEBUG_SIZE, do it later, after padmod(...) has printed the per-module infos. */
    checksize();
#endif

    setsym("_etext", etextoffset);
    setsym("_edata", edataoffset);
    setsym("_end", endoffset);

    openout(outfilename);
    writeheader();
    for (modptr = modfirst; modptr != (struct modstruct*) 0; modptr = modptr->modnext)
	if (modptr->loadflag)
	{
	    linkmod(modptr);
	    padmod(modptr);
	}
#ifdef DEBUG_SIZE
    checksize();
#endif

    /* dump symbol table */
    if (!stripflag)
    {
	seekout((unsigned INT32T) ((unsigned INT32T) A_MINHDR + (unsigned INT32T) (etextpadoff - btextoffset) + (unsigned INT32T) (edataoffset - bdataoffset)));
	memset(extsym + OFFSETOF_n_unused, 0, A_LISTHDR - OFFSETOF_n_unused);  /* .n_numaux = .n_type = 0; */  /* Unused fields. */
	for (modptr = modfirst; modptr != (struct modstruct*) 0; modptr = modptr->modnext)
	    if (modptr->loadflag)
	    {
		REGISTER struct symstruct **symparray;
		REGISTER struct symstruct *symptr;

		for (symparray = modptr->symparray;
		     (symptr = *symparray) != (struct symstruct*) 0; ++symparray)
		    if (symptr->modptr == modptr)
		    {
		        namecpy(extsym + OFFSETOF_n_name, extsym + OFFSETOF_n_name + 8, symptr->name);
			u4c4(extsym + OFFSETOF_n_value, (u4_pt) symptr->value);
#if 0
			/* 0x4063 == 040143 == ((3 & SEGM_MASK) | C_MASK | I_MASK | (1 << SZ_SHIFT). */
			if (strcmp(symptr->name, "_environ") == 0) printf("sym %s flags=0x%x=0%o\n", symptr->name, symptr->flags, symptr->flags);
#endif
			if ((flags = symptr->flags) & A_MASK)
			    extsym[OFFSETOF_n_sclass] = N_ABS;  /* 01 for Minix. */
#ifdef MINIXBUGCOMPAT
			else if (flags & (E_MASK | I_MASK) && !(flags & C_MASK))  /* The old ld linker in bccbin32.tar.Z had this bug: it incorrectly marked _environ (with C_MASK) as C_STAT rather than C_EXT. */
#else
			else if (flags & (E_MASK | I_MASK))
#endif
			    extsym[OFFSETOF_n_sclass] = C_EXT;  /* 020 == 0x10 for Minix. */
			else
			    extsym[OFFSETOF_n_sclass] = C_STAT;  /* 030 == 0x18 for Minix. */
			if (!(flags & I_MASK) ||
			     flags & C_MASK)
			    switch (flags & (A_MASK | SEGM_MASK))
			    {
			    case 0:
				extsym[OFFSETOF_n_sclass] |= N_TEXT;  /* 02 for Minix. */
			    case A_MASK:
				break;
			    default:
				if (flags & (C_MASK | SA_MASK))
				    extsym[OFFSETOF_n_sclass] |= N_BSS;  /* 04 for Minix. */
				else
				    extsym[OFFSETOF_n_sclass] |= N_DATA;  /* 03 for Minix. */
				break;
			    }
			writeout(extsym, sizeof extsym);
			++nsym;
		    }
	    }
	seekout((unsigned INT32T) OFFSETOF_a_syms);
	u4c4(buf4, (u4_pt) ((u4_t) nsym * sizeof extsym));
	writeout(buf4, 4);
    }
    closeout();
    executable();
}

PRIVATE void linkmod P1(struct modstruct *, modptr)
{
    char buf[ABS_TEXT_MAX];
    int command;
    unsigned char modify;
    offset_t offset;
    int symbolnum;
    struct symstruct **symparray;
    struct symstruct *symptr;

    setseg(0);
    relocsize = 2;
    symparray = modptr->symparray;
    openin(modptr->filename);	/* does nothing if already open */
    seekin((unsigned INT32T) modptr->textoffset);
    while (TRUE)
    {
	if ((command = readchar()) < 0)
	    prematureeof();
	modify = command & MODIFY_MASK;
	switch (command & CM_MASK)
	{
	case CM_SPECIAL:
	    switch (modify)
	    {
	    case CM_EOT:
		segpos[curseg] = spos;
		return;
	    case CM_BYTE_SIZE:
		relocsize = 1;
		break;
	    case CM_WORD_SIZE:
		relocsize = 2;
		break;
	    case CM_LONG_SIZE:
		relocsize = 4;
		break;
	    case CM_1_SKIP:
		skip(1);
		break;
	    case CM_2_SKIP:
		skip(2);
		break;
	    case CM_4_SKIP:
		skip(4);
		break;
	    default:
		if ((modify -= CM_0_SEG) >= NSEG)
		    inputerror("bad data in");
		setseg((unsigned) modify);
		break;
	    }
	    break;
	case CM_ABSOLUTE:
	    if (modify == 0)
		modify = ABS_TEXT_MAX;
	    readin(buf, (unsigned) modify);
	    writeout(buf, (unsigned) modify);
	    spos += (int) modify;
	    break;
	case CM_OFFSET_RELOC:
	    offset = readsize(relocsize);
	    if (modify & R_MASK)
		offset -= (spos + relocsize);
	    u4cn(buf, (u4_pt) (segbase[modify & SEGM_MASK] + offset), relocsize);
	    writeout(buf, relocsize);
	    spos += relocsize;
	    break;
	case CM_SYMBOL_RELOC:
	    symptr = symparray[symbolnum = readconvsize((unsigned)
					    (modify & S_MASK ? 2 : 1))];
	    offset = readconvsize((unsigned) modify & OF_MASK);
	    if (modify & R_MASK)
		offset -= (spos + relocsize);
	    offset += symptr->value;	    
	    u4cn(buf, (u4_pt) offset, relocsize);
	    writeout(buf, relocsize);
	    spos += relocsize;
	}
    }
}

PRIVATE void linkrefs P1(struct modstruct *, modptr)
{
    REGISTER struct symstruct **symparray;
    REGISTER struct symstruct *symptr;

    modptr->loadflag = TRUE;
    for (symparray = modptr->symparray;
	 (symptr = *symparray) != (struct symstruct*) 0; ++symparray)
	if (symptr->modptr->loadflag == FALSE)
	    linkrefs(symptr->modptr);
}

PRIVATE void padmod P1(struct modstruct *, modptr)
{
    offset_t count;
    unsigned seg;
    offset_t size;
    unsigned sizecount;
    char *sizeptr;

#ifdef DEBUG_SIZE_NOPAD
    /* Please note that a_bss values don't add up, because common symbols (C_MASK, N_COMM) may be defined in multiple modules. */
    fprintf(stderr, "info: module size: a_text=%lu a_data=%lu a_bss=%lu f=%s%s%s\n", (unsigned long) (segpos[0] - segbase[0]), (unsigned long) (segpos[3] - segbase[3]), (unsigned long) modptr->modcomsz, modptr->filename, modptr->archentry ? "//" : "", modptr->archentry ? modptr->archentry : "");
    fflush(stderr);
#endif
    for (seg = 0, sizeptr = modptr->segsize; seg < NSEG; ++seg)
    {
	size = cnu4(sizeptr, sizecount = segsizecount((unsigned) seg, modptr));
	sizeptr += sizecount;
	if ((count = segpos[seg] - segbase[seg]) != size)
	    size_error(seg, count, size);

	/* pad to quad boundary */
	/* not padding in-between common areas which sometimes get into file */
	if ((size = ldroundup(segpos[seg], 4, offset_t) - segpos[seg]) != 0)
	{
	    setseg(seg);
	    writenulls(size);
	    segpos[seg] = spos;
	}
#ifndef DEBUG_SIZE
	segbase[seg] = segpos[seg];
#endif
    }
#ifdef DEBUG_SIZE
#  ifndef DEBUG_SIZE_NOPAD
    /* This reports the padded size, because ldroundup has been called above. */
    /* Please note that a_bss values don't add up, because common symbols (C_MASK, N_COMM) may be defined in multiple modules. */
    fprintf(stderr, "info: module size: a_text=%lu a_data=%lu a_bss=%lu f=%s%s%s\n", (unsigned long) (segpos[0] - segbase[0]), (unsigned long) (segpos[3] - segbase[3]), (unsigned long) modptr->modcomsz, modptr->filename, modptr->archentry ? "//" : "", modptr->archentry ? modptr->archentry : "");
    fflush(stderr);
#  endif
    for (seg = 0; seg < NSEG; ++seg)
    {
	segbase[seg] = segpos[seg];
    }
#endif
}

PRIVATE void setsym P2(_CONST char *, name, offset_t, value)
{
    struct symstruct *symptr;

    if ((symptr = findsym(name)) != (struct symstruct*) 0)
	symptr->value = value;
}

PRIVATE void symres P1(REGISTER _CONST char *, name)
{
    REGISTER struct symstruct *symptr;

    if ((symptr = findsym(name)) != (struct symstruct*) 0)
    {
	if ((symptr->flags & SEGM_MASK) == SEGM_MASK)
	    symptr->flags &= ~SEGM_MASK | curseg;
	if (symptr->flags != (I_MASK | curseg) || symptr->value != 0)
	    reserved(name);
	symptr->flags = E_MASK | curseg;	/* show defined, not common */
    }
}

/* set new segment */

PRIVATE void setseg P1(unsigned, newseg)
{
    if (newseg != curseg)
    {
	segpos[curseg] = spos;
	spos = segpos[curseg = newseg];
	seekout((unsigned INT32T) ((unsigned INT32T) A_MINHDR + (unsigned INT32T) spos + (unsigned INT32T) segadj[curseg]));
    }
}

PRIVATE void skip P1(unsigned, countsize)
{
    writenulls((offset_t) readsize(countsize));
}


PRIVATE void writeheader P0()
{
    char header[A_MINHDR];  /* Minix a.out executable header. */
    offset_t a_total;

    memset(header, 0, sizeof header);  /* Some fields are unfilled, we zero-initialize them. */
    header[0] = 1;  /* a_magic[0]. */
    header[1] = 3;  /* a_magic[1]. */
    header[2] = (sepid ? A_SEP : A_EXEC) | (uzp ? A_UZP : 0);  /* a_flags. */
    header[3] = bits32 ? A_I80386 : A_I8086;  /* a_cpu. */
    header[4] = A_MINHDR;  /* a_hdrlen. */
    u4c4(header + OFFSETOF_a_text, (u4_pt) (etextpadoff - btextoffset));
    u4c4(header + OFFSETOF_a_data, (u4_pt) (edataoffset - bdataoffset));
    u4c4(header + OFFSETOF_a_bss, (u4_pt) (endoffset - edataoffset));
    if (uzp) u4c4(header + OFFSETOF_a_entry, (u4_pt) page_size());

    if (dynam_size) { a_total = endoffset + dynam_size; }
#ifdef MINIXBUGCOMPAT
    else if (endoffset < (offset_t) 0x10000L) { a_total = (offset_t) 0x10000L; }
#else  /* The ACK C compiler in Minix 1.5.10 i86 targeting Minix 1.5.10 i86 always sets a_total to 0x10000, so we do the same for !bits32. */
    else if (!bits32 || endoffset <= (offset_t) (0x10000L - 0x8000L)) { a_total = (offset_t) 0x10000L; }
#endif
    else { a_total = endoffset + (offset_t) 0x8000L; }
    if (a_total < endoffset || a_total > (bits32 ? (offset_t) ~a_total : (offset_t) 0x10000L)) {
	fatalerror(dynam_size ? "dynamic memory size (-h) makes a_total too large" : "a_total too large");
    }
    u4c4(header + OFFSETOF_a_total, (u4_pt) a_total);

    writeout(header, A_MINHDR);
}

PRIVATE void writenulls P1(offset_t, count)
{
    spos += count;
    while (count--)
	writechar(0);
}
