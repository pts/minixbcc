/* readobj.c - read object file for linker */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <string.h>
#endif
#include "const.h"
#include "obj.h"
#include "type.h"
#include "globvar.h"

#define ARMAG	"!<arch>\n"  /* Header bytes (8, trailing NUL unused). */
#define SARMAG	8  /* Number of header bytes. */
struct ar_hdr {  /* GNU archive (*.a) member header. */
	char	ar_name[16];  /* Space-padded. May be terminated by '/'. */
	char	ar_date[12];  /* Unix timestamp, decimal, space-padded. */
	char	ar_uid[6];  /* User  ID, decimal, space-padded. */
	char	ar_gid[6];  /* Group ID, decimal, space-padded. */
	char	ar_mode[8];  /* Stat st_mode, octal, space-padded. */
	char	ar_size[10];  /* File size in bytes, decimal, space-padded. */
	char	ar_fmag[2];  /* '`', '\n'. */
};
typedef char assert_sizeof_ar_hdr[sizeof(struct ar_hdr) == 60 ? 1 : -1];

#define	MINIXARMAG	0177545
#define MINIXARNAMEMAX    14
struct minixar_hdr {  /* Minix archive (*.a) member header. */
	char	ar_name[MINIXARNAMEMAX];
	char	ar_date[4];	/* mtime; int32_t in PDP-11 byte order 2 3 1 0. */
	char	ar_uid[1];	/* uint8_t. */
	char	ar_gid[1];	/* uint8_t. */
	char	ar_mode[2];	/* uint16_t in little-endian byte order 0 1. */
	char	ar_size[4];	/* int32_t in PDP-11 byte order 2 3 1 0. */
};
typedef char assert_sizeof_minixar_hdr[sizeof(struct minixar_hdr) == 26 ? 1 : -1];

/*
   Linking takes 2 passes. The 1st pass reads through all files specified
in the command line, and all libraries. All public symbols are extracted
and stored in a chained hash table. For each module, its file and header
data recorded, and the resulting structures are chained together
(interleaved with the symbols).

   The symbol descriptors are separated from the symbol names, so we must
record all the descriptors of a module before putting the symbols in the
symbol table (poor design). The descriptors are stored in the symbol
table, then moved to the top of the table to make room for the symols.
The symbols referred to in a given module are linked together by a chain
beginning in the module descriptor.
*/

PUBLIC struct entrylist *entryfirst;	/* first on list of entry symbols */
PUBLIC struct modstruct *modfirst;	/* data for 1st module */
PUBLIC struct redlist *redfirst;	/* first on list of redefined symbols */

PRIVATE char convertsize[NSEG / 4] = {0, 1, 2, 4};
PRIVATE struct entrylist *entrylast;	/* last on list of entry symbols */
PRIVATE struct redlist *redlast;	/* last on list of redefined symbols */
PRIVATE struct modstruct *modlast;	/* data for last module */

PRIVATE char *namedup P((char *p, unsigned size));
PRIVATE bool_pt readarheader P((char **parchentry, offset_t *filelength_out));
PRIVATE bool_pt readminixarheader P((char **parchentry, offset_t *filelength_out));
PRIVATE unsigned read1fileheader1 P((void));
PRIVATE unsigned read2fileheader2 P((void));
PRIVATE void readmodule P((char *filename, char *archentry));
PRIVATE void reedmodheader P((void));
PRIVATE bool_pt redsym P((struct symstruct *symptr, offset_t value));
PRIVATE unsigned checksum P((char *string, unsigned length));
PRIVATE unsigned segbits P((unsigned seg, char *sizedesc));
PRIVATE unsigned readfilecommon P((char *fileheader));

/* initialise object file handler */

PUBLIC void objinit()
{
    modfirst = modlast = (struct modstruct*) 0;
    entryfirst = entrylast = (struct entrylist*) 0;
    redfirst = redlast = (struct redlist*) 0;
}

/* read all symbol definitions in an object file */

PUBLIC void readsyms(filename)
char *filename;
{
    char *archentry;
    char filemagic[SARMAG];
    offset_t filepos;
    offset_t filelength;
    unsigned modcount;

    openin(filename);		/* input is not open, so position is start */
    switch ((unsigned) readsize(2))
    {
    case OMAGIC:
	for (modcount = read2fileheader2(); modcount-- != 0;)
	    readmodule(filename, (char *) 0);
	break;
    case MINIXARMAG:
	filepos = 2;
	while (readminixarheader(&archentry, &filelength))
	{
	    filepos += sizeof(struct minixar_hdr);
	    for (modcount = read1fileheader1(); modcount-- != 0;)
	    {
		readmodule(stralloc(filename), archentry);
		modlast->textoffset += filepos;
	    }
	    seekin((unsigned INT32T) (filepos += roundup(filelength, 2, offset_t)));
	}
	break;
    default:
	seekin((INT32T) 0);
	readin(filemagic, sizeof filemagic);
	if (memcmp(filemagic, ARMAG, sizeof filemagic) != 0)
	    input1error(" has bad magic number");
	filepos = SARMAG;
	while (readarheader(&archentry, &filelength))
	{
	    filepos += sizeof(struct ar_hdr);
	    for (modcount = read1fileheader1(); modcount-- != 0;)
	    {
		readmodule(stralloc(filename), archentry);
		modlast->textoffset += filepos;
	    }
	    seekin((unsigned INT32T) (filepos += roundup(filelength, 2, offset_t)));
	}
	break;
    }
    closein();
}

/* read archive header and return length */

/* Parses an unsigned decimal, hexadecimal or octal number. It doesn't check
 * for overflow. It allows leading ' ' and '\t'. It allows trailing ' ' and
 * '\t'. It fails if the result is negative. Returns nonzero on success. It
 * produces a twos complement number if the input has a minus sign.
 *
 * It uses the speified base (8, 10 or 16), or if 0 is specified, then it
 * autodetects the base just like c does.
 */
bool_pt parse_nonneg_lenient(s, base, output)
char *s;
unsigned base;
offset_t *output;
{
    register char c;
    bool_t sign;
    offset_t v;  /* Unsigned. */

    sign = 0;
    v = 0;
    for (; (c = *s) == ' ' || c == '\t'; ++s) {}
    if (*s == '-') { ++s; sign = 1; }
    if (*s == '0' && ((c = s[1]) == 'x' || c == 'X') && (base == 0 || base == 16)) {
	s += 2;
	base = 16;
    } else if (*s == '0' && (base == 0 || base == 8)) {
	base = 8;
    } else if (base == 10) {
    } else if (base == 0) {
	base = 10;
    } else {
	return 0;    /* Invalid base. */
    }
    c = *s++;
    do {
	if (c >= '0' && c <= '9' && base == 10) {    /* This doesn't work in EBCDIC, it's probably ASCII-only. */
	    c -= '0';
	    v *= 10;
	} else if (c >= '0' && c <= '7' && base == 8) {
	    c -= '0';
	    v <<= 3;
	} else if (c >= '0' && c <= '9' && base == 16) {    /* This doesn't work in EBCDIC, it's probably ASCII-only. */
	    c -= '0';
	    v <<= 4;
	} else if (c >= 'a' && c <= 'f' && base == 16) {
	    c -= 'a' - 10;
	    v <<= 4;
	} else if (c >= 'A' && c <= 'F' && base == 16) {
	    c -= 'A' - 10;
	    v <<= 4;
	} else {
	    return 0;    /* Bad digit in c. */
	}
#ifdef __BCC__
#  ifdef __AS386_32__
	v += (unsigned long) (unsigned char) c;  /* Cast to (unsigned long) Works around suboptimal code generation in BCC sc v3. */
#  else
	v += (unsigned char) c;
#  endif
#else
	v += (unsigned char) c;
#endif
    } while ((c = *s++) != '\0' && c != ' ' && c != '\t');
    for (; c == ' ' || c == '\t'; c = *s++) {}
    if (c != '\0') return 0;    /* Trailing garbage. */
    if (sign) v = ~v + 1;    /* Two's complement. */
    if (v > ~v) return 0;    /* Negative two's complement. */
    *output = v;  /* Always nonnegative here. */
    return 1;
}

PRIVATE char *namedup(p, size)
char *p;
unsigned size;
{
    char *pend, *result;
    register char *q;

    for (pend = (q = p) + size; q != pend && *q != '\0' && *q != '/'; ++q) {}
    for (; q != p && q[-1] == ' '; --q) {}  /* Remove trailing spaces. */
    size = q - p;
    memcpy(result = heapalloc(size + 1), p, size);
    result[size] = '\0';
    return result;
}

PRIVATE bool_pt readarheader(parchentry, filelength_out)
char **parchentry;
offset_t *filelength_out;
{
    struct ar_hdr arheader;

    if (readineofok((char *) &arheader, sizeof arheader))
	return 0;
    *parchentry = namedup(arheader.ar_name, sizeof arheader.ar_name);
    return parse_nonneg_lenient(arheader.ar_size, 10, filelength_out);  /* Always ASCII decimal. */
}

PRIVATE bool_pt readminixarheader(parchentry, filelength_out)
char **parchentry;
offset_t *filelength_out;
{
    struct minixar_hdr marheader;

    if (readineofok((char *) &marheader, sizeof marheader))
	return 0;
    *parchentry = namedup(marheader.ar_name, sizeof marheader.ar_name);
    *filelength_out = (offset_t) c2u2(marheader.ar_size) << 16 | c2u2(marheader.ar_size + 2);  /* PDP-11 byte order. */
    return 1;
}

/* read and check file header of the object file just opened */

PRIVATE unsigned readfilecommon(fileheader)
char *fileheader;
{
    char filechecksum;		/* part of fileheader but would unalign */

    readin(&filechecksum, sizeof filechecksum);
    if (filechecksum != (char) checksum(fileheader, 4))
	input1error(" is not an object file");
    return c2u2(fileheader + 2  /* .count */);
}

PRIVATE unsigned read1fileheader1()
{
    struct
    {
	char magic[2];
	char count_ca[2];	/* really an int */
    } fileheader;

    readin((char *) &fileheader, sizeof fileheader);
    return readfilecommon((char *) &fileheader);
}

PRIVATE unsigned read2fileheader2()
{
    struct
    {
	char magic[2];
	char count_ca[2];	/* really an int */
    } fileheader;

    fileheader.magic[0] = (char) (OMAGIC & 0xff);  /* !! Pre-add these 2 bytes, pass the result to readfilecommon. */
    fileheader.magic[1] = (char) ((unsigned) OMAGIC >> 8);
    readin(fileheader.count_ca, 2);
    return readfilecommon((char *) &fileheader);
}

/* read the next module */

PRIVATE void readmodule(filename, archentry)
char *filename;
char *archentry;
{
    struct symdstruct		/* to save parts of symbol before name known */
    {
	offset_t dvalue;
	flags_t dflags;
    };
    struct symdstruct *endsymdptr;
    flags_t flags;
    unsigned nsymbol;
    struct symdstruct *symdptr;
    char *symname;
    struct symstruct **symparray;
    struct symstruct *symptr;

    reedmodheader();
    modlast->filename = filename;
    modlast->archentry = archentry;
    nsymbol = readsize(2);
    symdptr = (struct symdstruct *)
	heapalloc(nsymbol * sizeof(struct symdstruct));
    for (endsymdptr = symdptr + nsymbol; symdptr < endsymdptr; ++symdptr)
    {
	readsize(2);		/* discard string offset, assume strings seq */
	symdptr->dflags = flags = readsize(2);
	symdptr->dvalue = readconvsize((flags & SZ_MASK) >> SZ_SHIFT);
	/* NB unsigned flags to give logical shift */
	/* bug in Xenix 2.5 cc causes (int) of the */
	/* argument to turn flags into an int */
    }
    symdptr = (struct symdstruct *)
	moveup(nsymbol * sizeof(struct symdstruct));
    modlast->symparray = symparray = (struct symstruct **)
	heapalloc((nsymbol + 1) * sizeof(struct symstruct *));
    symname = readstring();	/* module name */
    modlast->modname = stralloc(symname);	/* likely OK overlapped copy */
    for (endsymdptr = symdptr + nsymbol; symdptr < endsymdptr;
	 *symparray++ = symptr, release((char *) ++symdptr))
    {
	symname = readstring();
	if ((flags = symdptr->dflags) & (E_MASK | I_MASK) &&
	    (symptr = findsym(symname)) != (struct symstruct*) 0)
	{
	    /*
	       weaken segment-checking by letting the maximum segment
	       (SEGM_MASK) mean any segment
	    */
	    if ((symptr->flags & SEGM_MASK) == SEGM_MASK)
		symptr->flags &= ~SEGM_MASK | (flags & SEGM_MASK);
	    else if ((flags & SEGM_MASK) == SEGM_MASK)
		flags &= ~SEGM_MASK | (symptr->flags & SEGM_MASK);
	    if ((flags ^ symptr->flags) & (N_MASK | A_MASK | SEGM_MASK))
	    {
		redefined(symname, " with different segment or relocatability",
			  archentry, symptr->modptr->filename,
			  symptr->modptr->archentry);
		continue;
	    }
	    if (symptr->flags & E_MASK)
	    {
		if (flags & E_MASK && redsym(symptr, symdptr->dvalue))
		    redefined(symname, "", archentry, symptr->modptr->filename,
			      symptr->modptr->archentry);
		continue;
	    }
	    if (flags & I_MASK && symdptr->dvalue <= symptr->value)
		continue;
	}
	else
	    symptr = addsym(symname);
	symptr->modptr = modlast;
	symptr->value = symdptr->dvalue;
	symptr->flags = flags;
	if (flags & N_MASK)
	    entrysym(symptr);
    }
    *symparray = (struct symstruct*) 0;
}

/* put symbol on entry symbol list if it is not already */

PUBLIC void entrysym(symptr)
struct symstruct *symptr;
{
    register struct entrylist *elptr;

    for (elptr = entryfirst; elptr != (struct entrylist*) 0; elptr = elptr->elnext)
	if (symptr == elptr->elsymptr)
	    return;
    elptr = (struct entrylist *) heapalloc(sizeof(struct entrylist));
    elptr->elnext = (struct entrylist*) 0;
    elptr->elsymptr = symptr;
    if (entryfirst == (struct entrylist*) 0)
	entryfirst = elptr;
    else
	entrylast->elnext = elptr;
    entrylast = elptr;
}

/* read the header of the next module */

PRIVATE void reedmodheader()
{
    struct
    {
	char htextoffset[4];	/* offset to module text in file */
	char htextsize[4];	/* size of text (may be 0 for last mod) */
	char stringssize[2];	/* size of string area */
	char hclass;		/* module class */
	char revision;		/* module revision */
    }
     modheader;
    unsigned seg;
    unsigned count;
    char *cptr;
    struct modstruct *modptr;

    readin((char *) &modheader, sizeof modheader);
    modptr = (struct modstruct *) heapalloc(sizeof(struct modstruct));
    modptr->modnext = (struct modstruct*) 0;
    modptr->textoffset = c4u4(modheader.htextoffset);
    modptr->class = modheader.hclass;
    readin(modptr->segmaxsize, sizeof modptr->segmaxsize);
    readin(modptr->segsizedesc, sizeof modptr->segsizedesc);
    cptr = modptr->segsize;
    for (seg = 0; seg < NSEG; ++seg)
    {
	if ((count = segsizecount(seg, modptr)) != 0)
	{
	    if (cptr == modptr->segsize)
		heapalloc(count - 1);	/* 1st byte reserved in struct */
	    else
		heapalloc(count);
	    readin(cptr, count);
	    cptr += count;
	}
    }
    if (modfirst == (struct modstruct*) 0)
	modfirst = modptr;
    else
	modlast->modnext = modptr;
    modlast = modptr;
}

PRIVATE bool_pt redsym(symptr, value)
register struct symstruct *symptr;
offset_t value;
{
    register struct redlist *rlptr;
    char class;

    if (symptr->modptr->class != (class = modlast->class))
	for (rlptr = redfirst;; rlptr = rlptr->rlnext)
	{
	    if (rlptr == (struct redlist*) 0)
	    {
		rlptr = (struct redlist *)
		    heapalloc(sizeof(struct redlist));
		rlptr->rlnext = (struct redlist*) 0;
		rlptr->rlsymptr = symptr;
		if (symptr->modptr->class < class)
		    /* prefer lower class - put other on redlist */
		{
		    rlptr->rlmodptr = modlast;
		    rlptr->rlvalue = value;
		}
		else
		{
		    rlptr->rlmodptr = symptr->modptr;
		    symptr->modptr = modlast;
		    rlptr->rlvalue = symptr->value;
		    symptr->value = value;
		}
		if (redfirst == (struct redlist*) 0)
		    redfirst = rlptr;
		else
		    redlast->rlnext = rlptr;
		redlast = rlptr;
		return FALSE;
	    }
	    if (symptr == rlptr->rlsymptr && class == rlptr->rlmodptr->class)
		break;
	}
    return TRUE;
}

PRIVATE unsigned checksum(string, length)
char *string;
unsigned length;
{
    unsigned char sum;		/* this is a 1-byte checksum */

    for (sum = 0; length-- != 0;)
	sum += *string++ & 0xFF;
    return sum;
}

PUBLIC offset_t readconvsize(countindex)
unsigned countindex;
{
    return readsize(convertsize[countindex]);
}

PUBLIC offset_t readsize(count)
unsigned count;
{
    char buf[MAX_OFFSET_SIZE];

    if (count == 0)
	return 0;
    readin(buf, count);
    return cnu4(buf, count);
}

PRIVATE unsigned segbits(seg, sizedesc)
unsigned seg;
char *sizedesc;
{
    return 3 & ((unsigned) sizedesc[((NSEG - 1) - seg) / 4] >> (2 * (seg % 4)));
    /* unsigned to give logical shift */
}

PUBLIC unsigned segsizecount(seg, modptr)
unsigned seg;
struct modstruct *modptr;
{
    return convertsize[segbits(seg, modptr->segsizedesc)];
}
