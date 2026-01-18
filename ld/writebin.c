/* writebin.c - write binary file for linker */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <string.h>
#endif
#include "aout.h"
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
#endif

/* After aout.h */
#define a_entry a_no_entry
#define n_was_name n_name
#define n_was_numaux n_numaux
#define n_was_other n_other
#define n_was_sclass n_sclass
#define n_was_strx n_value
#define n_was_type n_type

#define page_size() 4096

#define FILEHEADERLENGTH A_MINHDR
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

#define offsetof(struc, mem) ((int) &((struc *) 0)->mem)
#define memsizeof(struc, mem) sizeof(((struc *) 0)->mem)

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
FORWARD void setsym P((char *name, offset_t value));
FORWARD void symres P((char *name));
FORWARD void setseg P((unsigned newseg));
FORWARD void skip P((unsigned countsize));
FORWARD void writeheader P((void));
FORWARD void writenulls P((offset_t count));

/* link all symbols connected to entry symbols */

PUBLIC void linksyms(argreloc_output)
bool_pt argreloc_output;
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
		    rlptr->rlmodptr->class > rlptr->rlsymptr->modptr->class)
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

PRIVATE void checksize() {
    if (!bits32) {
	if (etextoffset > 0xff00L) fatalerror("a_text too large");  /* This is the actual limit for a_text in Minix 1.5.10 i86. */
	if (endoffset > 0xffc0L) fatalerror("a_data+a_bss too large");  /* 0x20 bytes for the stack. */
    }
}

PRIVATE void namecpy(p, pend, src)
char *p;
char *pend;
char *src;
{
	register char *q;

	for (q = p; q != pend && (*q++ = *src++) != '\0'; ) {}
	for (; q != pend; *q++ = '\0') {}
}

#ifdef ACKFIX  /* Helper function used below. */
PRIVATE offset_t *addcomseg(symptr)
struct symstruct *symptr;
{
    register offset_t *comszseg;

    symptr->value = *(comszseg = &comsz[symptr->flags & SEGM_MASK]);
    return comszseg;
}
#endif

/* write binary file */

PUBLIC void writebin(outfilename, argsepid, argbits32, argstripflag, arguzp)
char *outfilename;
bool_pt argsepid;
bool_pt argbits32;
bool_pt argstripflag;
bool_pt arguzp;
{
    char buf4[4];
    char *cptr;
#if 0  /* Defined in "aout.h" and Minix /usr/include/a.out.h . */
struct nlist {  /* symbol table entry */
  char            n_name[8];  /* symbol name */
  long            n_value;    /* value */
  unsigned char   n_sclass;   /* storage class */
  unsigned char   n_numaux;   /* number of auxiliary entries;  not used */
  unsigned short  n_type;     /* language base and derived type; not used */
};
/* low bits of storage class (section) */
#define N_SECT  07  /* section mask */
#define N_UNDF  00  /* undefined */
#define N_ABS   01  /* absolute */
#define N_TEXT  02  /* text */
#define N_DATA  03  /* data */
#define N_BSS   04  /* bss */
#define N_COMM  05  /* (common) */

/* high bits of storage class */
#define N_CLASS 0370  /* storage class mask */
#define C_NULL
#define C_EXT   0020  /* external symbol */
#define C_STAT  0030  /* static */
#endif
    struct nlist extsym;  /* !! Don't use this struct, to avoid alignment issues during serialization. */
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
	    register struct symstruct **symparray;
	    register struct symstruct *symptr;

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
			    tempoffset = roundup(symptr->value, 4, offset_t);
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
		segsz[seg] += cntooffset(cptr,
			  sizecount = segsizecount((unsigned) seg, modptr));

		/* adjust sizes to even to get quad boundaries */
		/* this should be specifiable dynamically */
		segsz[seg] = roundup(segsz[seg], 4, offset_t);
		comsz[seg] = roundup(comsz[seg], 4, offset_t);
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
	etextpadoff = roundup(etextoffset, 0x10, offset_t);
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
	    register struct symstruct **symparray;
	    register struct symstruct *symptr;

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
    fprintf(stderr, "info: total executable size: a_text=%lu a_data=%lu a_bss=%lu a_data+a_bss=%lu\n", ETEXTMAYBEPADOFF - btextoffset, edataoffset - bdataoffset, endoffset - edataoffset, endoffset);
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
	seekout(FILEHEADERLENGTH
		+ (long) (etextpadoff - btextoffset)
		+ (long) (edataoffset - bdataoffset)
		);
	extsym.n_was_numaux = extsym.n_was_type = 0;
	for (modptr = modfirst; modptr != (struct modstruct*) 0; modptr = modptr->modnext)
	    if (modptr->loadflag)
	    {
		register struct symstruct **symparray;
		register struct symstruct *symptr;

		for (symparray = modptr->symparray;
		     (symptr = *symparray) != (struct symstruct*) 0; ++symparray)
		    if (symptr->modptr == modptr)
		    {
		        namecpy(extsym.n_was_name, extsym.n_was_name + sizeof extsym.n_was_name, symptr->name);
			u4cn((char *) &extsym.n_value, (u4_t) symptr->value,
			     sizeof extsym.n_value);
#if 0
			/* 0x4063 == 040143 == ((3 & SEGM_MASK) | C_MASK | I_MASK | (1 << SZ_SHIFT). */
			if (strcmp(symptr->name, "_environ") == 0) printf("sym %s flags=0x%x=0%o\n", symptr->name, symptr->flags, symptr->flags);
#endif
			if ((flags = symptr->flags) & A_MASK)
			    extsym.n_was_sclass = N_ABS;  /* 01 for Minix. */
#ifdef MINIXBUGCOMPAT
			else if (flags & (E_MASK | I_MASK) && !(flags & C_MASK))  /* The old ld linker in bccbin32.tar.Z had this bug: it incorrectly marked _environ (with C_MASK) as C_STAT rather than C_EXT. */
#else
			else if (flags & (E_MASK | I_MASK))
#endif
			    extsym.n_was_sclass = C_EXT;  /* 020 == 0x10 for Minix. */
			else
			    extsym.n_was_sclass = C_STAT;  /* 030 == 0x18 for Minix. */
			if (!(flags & I_MASK) ||
			     flags & C_MASK)
			    switch (flags & (A_MASK | SEGM_MASK))
			    {
			    case 0:
				extsym.n_was_sclass |= N_TEXT;  /* 02 for Minix. */
			    case A_MASK:
				break;
			    default:
				if (flags & (C_MASK | SA_MASK))
				    extsym.n_was_sclass |= N_BSS;  /* 04 for Minix. */
				else
				    extsym.n_was_sclass |= N_DATA;  /* 03 for Minix. */
				break;
			    }
			writeout((char *) &extsym, sizeof extsym);
			++nsym;
		    }
	    }
	seekout((long) offsetof(struct exec, a_syms));
	u4cn(buf4, (u4_t) nsym * sizeof extsym,
	     memsizeof(struct exec, a_syms));
	writeout(buf4, memsizeof(struct exec, a_syms));
    }
    closeout();
    executable();
}

PRIVATE void linkmod(modptr)
struct modstruct *modptr;
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
    seekin((long) modptr->textoffset);
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
#ifdef LONG_OFFSETS
		relocsize = 4;
		break;
#else
		fatalerror("relocation by long offsets not implemented");
#endif
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
	    offtocn(buf, segbase[modify & SEGM_MASK] + offset, relocsize);
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
	    offtocn(buf, offset, relocsize);
	    writeout(buf, relocsize);
	    spos += relocsize;
	}
    }
}

PRIVATE void linkrefs(modptr)
struct modstruct *modptr;
{
    register struct symstruct **symparray;
    register struct symstruct *symptr;

    modptr->loadflag = TRUE;
    for (symparray = modptr->symparray;
	 (symptr = *symparray) != (struct symstruct*) 0; ++symparray)
	if (symptr->modptr->loadflag == FALSE)
	    linkrefs(symptr->modptr);
}

PRIVATE void padmod(modptr)
struct modstruct *modptr;
{
    offset_t count;
    unsigned seg;
    offset_t size;
    unsigned sizecount;
    char *sizeptr;

#ifdef DEBUG_SIZE_NOPAD
    /* Please note that a_bss values don't add up, because common symbols (C_MASK, N_COMM) may be defined in multiple modules. */
    fprintf(stderr, "info: module size: a_text=%lu a_data=%lu a_bss=%lu f=%s%s%s\n", segpos[0] - segbase[0], segpos[3] - segbase[3], modptr->modcomsz, modptr->filename, modptr->archentry ? "//" : "", modptr->archentry ? modptr->archentry : "");
#endif
    for (seg = 0, sizeptr = modptr->segsize; seg < NSEG; ++seg)
    {
	size = cntooffset(sizeptr,
			  sizecount = segsizecount((unsigned) seg, modptr));
	sizeptr += sizecount;
	if ((count = segpos[seg] - segbase[seg]) != size)
	    size_error(seg, count, size);

	/* pad to quad boundary */
	/* not padding in-between common areas which sometimes get into file */
	if ((size = roundup(segpos[seg], 4, offset_t) - segpos[seg]) != 0)
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
    /* This reports the padded size (because roundup has been called above. */
    /* Please note that a_bss values don't add up, because common symbols (C_MASK, N_COMM) may be defined in multiple modules. */
    fprintf(stderr, "info: module size: a_text=%lu a_data=%lu a_bss=%lu f=%s%s%s\n", segpos[0] - segbase[0], segpos[3] - segbase[3], modptr->modcomsz, modptr->filename, modptr->archentry ? "//" : "", modptr->archentry ? modptr->archentry : "");
#  endif
    for (seg = 0; seg < NSEG; ++seg)
    {
	segbase[seg] = segpos[seg];
    }
#endif
}

PRIVATE void setsym(name, value)
char *name;
offset_t value;
{
    struct symstruct *symptr;

    if ((symptr = findsym(name)) != (struct symstruct*) 0)
	symptr->value = value;
}

PRIVATE void symres(name)
register char *name;
{
    register struct symstruct *symptr;

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

PRIVATE void setseg(newseg)
unsigned newseg;
{
    if (newseg != curseg)
    {
	segpos[curseg] = spos;
	spos = segpos[curseg = newseg];
	seekout(FILEHEADERLENGTH + (long) spos + (long) segadj[curseg]);
    }
}

PRIVATE void skip(countsize)
unsigned countsize;
{
    writenulls((offset_t) readsize(countsize));
}

#ifndef A_UZP
#  define A_UZP 1
#endif

PRIVATE void writeheader()
{
    struct exec header;  /* !! Don't use `struct exec' here, to avoid alignment issues. */
    offset_t a_total;

    memset(&header, 0, sizeof header);
    header.a_magic[0] = A_MAGIC0;
    header.a_magic[1] = A_MAGIC1;
    header.a_flags = sepid ? A_SEP : A_EXEC;
    if (uzp) header.a_flags |= A_UZP;
    header.a_cpu = bits32 ? A_I80386 : A_I8086;
    header.a_hdrlen = FILEHEADERLENGTH;
    offtocn((char *) &header.a_text, etextpadoff - btextoffset,
	    sizeof header.a_text);
    offtocn((char *) &header.a_data, edataoffset - bdataoffset,
	    sizeof header.a_data);
    offtocn((char *) &header.a_bss, endoffset - edataoffset,
	    sizeof header.a_bss);
    if (uzp) offtocn((char *) &header.a_entry, page_size(), sizeof header.a_entry);

    if (dynam_size) { a_total = endoffset + dynam_size; }
#ifdef MINIXBUGCOMPAT
    else if (endoffset < 0x10000L) { a_total = (offset_t) 0x10000L; }
#else  /* The ACK C compiler in Minix 1.5.10 i86 targeting Minix 1.5.10 i86 always sets a_total to 0x10000, so we do the same for !bits32. */
    else if (!bits32 || endoffset <= (offset_t) (0x10000L - 0x8000L)) { a_total = (offset_t) 0x10000L; }
#endif
    else { a_total = endoffset + (offset_t) 0x8000L; }
    if (a_total < endoffset || a_total > (bits32 ? ~a_total : (offset_t) 0x10000L)) {
	fatalerror(dynam_size ? "dynamic memory size (-h) makes a_total too large" : "a_total too large");
    }
    offtocn((char *) &header.a_total, a_total, sizeof header.a_total);

    writeout((char *) &header, FILEHEADERLENGTH);
}

PRIVATE void writenulls(count)
offset_t count;
{
    spos += count;
    while (count--)
	writechar(0);
}
