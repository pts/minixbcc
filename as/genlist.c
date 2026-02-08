/* genlist.c - generate listing and error reports for assembler */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#  include <unistd.h>
#  include <string.h>
#endif
#include "const.h"
#include "type.h"
#include "address.h"
#include "flag.h"
#include "file.h"
#include "globvar.h"
#include "macro.h"
#include "scan.h"
#include "source.h"

#define CODE_LIST_LENGTH (sizeof (struct code_listing_s) - 1)
				/* length of formatted code listing */
#define MAXERR 6		/* maximum errors listed per line */

struct error_s			/* to record error info */
{
    unsigned char errnum;
    unsigned char position;
};

/* code listing format */

struct code_listing_s
{
    union linum_macro_u
    {
	char linum[LINUM_LEN];
	struct
	{
	    char pad[1];
	    char mark[1];
	    char level[1];
	}
	 macro;
    }
     linum_or_macro;
    char padlinum[1];
    char lc[4];
    char padlc[1];
    char lprefix[2];
    char aprefix[2];
    char oprefix[2];
    char sprefix[2];
    char page[2];
    char opcode[2];
    char postb[2];
    char sib[2];
    char padopcode[1];
    char displ4[2];
    char displ3[2];
    char displ2[2];
    char displ1[2];
    char reldispl[1];
    char paddispl[1];
    char imm4[2];
    char imm3[2];
    char imm2[2];
    char imm1[2];
    char relimm[1];
    char padimm[1];
    char nullterm;
};

PRIVATE struct error_s errbuf[MAXERR];	/* error buffer */
PRIVATE unsigned char errcount;	/* # errors in line */
PRIVATE bool_t erroverflow;	/* set if too many errors on line */

FORWARD char *build_1hex_number P((unsigned num, char *where));
FORWARD void list1 P((fd_t fd));
FORWARD void listcode P((void));
FORWARD void listerrors P((void));
FORWARD void paderrorline P((unsigned nspaces));

/* format 1 byte number as 2 hex digits, return ptr to end */

PRIVATE char *build_1hex_number P2(unsigned, num, REGISTER char *, where)
{
    static char hexdigits[] = "0123456789ABCDEF";

    where[0] = hexdigits[(num & 0xff) >> 4];
    where[1] = hexdigits[num & 0xf];
    return where + 2;
}

/* format 2 byte number as 4 hex digits, return ptr to end */

PUBLIC char *build_2hex_number P2(unsigned, num, char *, where)
{
    return build_1hex_number(num, build_1hex_number(num >> 8, where));
}

/* format 2 byte number as decimal with given width (pad with leading '0's) */
/* return ptr to end */

PUBLIC char *build_number P3(unsigned, num, unsigned, width, REGISTER char *, where)
{
    static unsigned powers_of_10[] = {1, 10, 100, 1000, 10000,};
    unsigned char digit;
    unsigned char power;
    REGISTER unsigned power_of_10;

    power = 5;			/* actually 1 more than power */
    do
    {
	for (digit = '0', power_of_10 = (powers_of_10 - 1)[power];
	     num >= power_of_10; num -= power_of_10)
	    ++digit;
	if (power <= width)
	    *where++ = digit;
    }
    while (--power != 0);
    return where;
}

/* record number and position of error (or error buffer overflow) */

PUBLIC void error P1(error_pt, errnum)
{
    REGISTER struct error_s *errptr;
    REGISTER struct error_s *errptrlow;
    unsigned char position;

    if (errnum < MINWARN || warn.current)
    {
	if (errcount >= MAXERR)
	    erroverflow = TRUE;
	else
	{
	    position = symname - linebuf;
	    for (errptr = errbuf + errcount;
		 errptr > errbuf && errptr->position > position;
		 errptr = errptrlow)
	    {
		errptrlow = errptr - 1;
		errptr->errnum = errptrlow->errnum;
		errptr->position = errptrlow->position;
	    }
	    errptr->errnum = errnum;
	    errptr->position = position;
	    ++errcount;
	    if (errnum >= MINWARN)
		++totwarn;
	    else
		++toterr;
	}
    }
}

/* list 1 line to list file if any errors or flags permit */
/* list line to console as well if any errors and list file is not console */

PUBLIC void listline P0()
{
    if (!listpre)
    {
	if (errcount || (list.current && (!macflag || mcount != 0)) ||
	    (macflag && maclist.current))
	    list1(lstfil);
	if (errcount)
	{
	    if (lstfil != STDOUT)
		list1(STDOUT);
	    errcount = 0;
	    erroverflow = FALSE;
	}
    }
}

/* list 1 line unconditionally */

PRIVATE void list1 P1(fd_t, fd)
{
    innum = fd;
    listcode();
    (void)!write(innum, linebuf, lineptr - linebuf);
    writenl();
    if (errcount != 0)
	listerrors();
    listpre = TRUE;
}

/* list object code for 1 line */

PRIVATE void listcode P0()
{
    unsigned char count;
    struct code_listing_s *listptr;
    int numlength;
    char *numptr;

    listptr = (struct code_listing_s *) heapptr;
    memset((char *) listptr, ' ', sizeof *listptr);
    listptr->nullterm = 0;
    if (macflag)
    {
	listptr->linum_or_macro.macro.mark[0] = '+';
	listptr->linum_or_macro.macro.level[0] = maclevel + ('a' - 1);
    }
    else
    {
	numlength = LINUM_LEN;
	numptr = listptr->linum_or_macro.linum;
	if (infiln != infil0)
	{
	    *numptr++ = infiln - infil0 + ('a' - 1);
	    numlength = LINUM_LEN - 1;
	}
	build_number(linum, numlength, numptr);
    }
    if ((count = mcount) != 0 || popflags & POPLC)
	build_2hex_number((unsigned) lc, listptr->lc);
    if (popflags & POPLO)
    {
	if (popflags & POPLONG)
	    build_2hex_number((unsigned) (lastexp.offset >> 16),
			      listptr->displ4);
	if (popflags & POPHI)
	    build_2hex_number((unsigned) lastexp.offset, listptr->displ2);
	else
	    build_1hex_number((unsigned) lastexp.offset, listptr->displ1);
	if (lastexp.data & RELBIT)
	    listptr->reldispl[0] = '>';
    }
    else if (count != 0)
    {
	if (aprefix != 0)
	{
	    --count;
	    build_1hex_number(aprefix, listptr->aprefix);
	}
	if (oprefix != 0)
	{
	    --count;
	    build_1hex_number(oprefix, listptr->oprefix);
	}
	if (sprefix != 0)
	{
	    --count;
	    build_1hex_number(sprefix, listptr->sprefix);
	}
	if (page != 0)
	{
	    build_1hex_number(page, listptr->page);
	    --count;
	}
	build_1hex_number(opcode, listptr->opcode);
	--count;
	if (postb != 0)
	{
	    --count;
	    build_1hex_number(postb,
			      listptr->postb);
	}
	if (sib != NO_SIB)
	{
	    --count;
	    build_1hex_number(sib, listptr->sib);
	}
	if (count > 0)
	{
	    build_1hex_number((opcode_pt) lastexp.offset, listptr->displ1);
	    if (lastexp.data & RELBIT)
		listptr->reldispl[0] = '>';
	}
	if (count > 1)
	    build_1hex_number((opcode_pt) lastexp.offset >> 0x8,
			      listptr->displ2);
	if (count > 2)
	{
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x10),
			      listptr->displ3);
	    build_1hex_number((opcode_pt) (lastexp.offset >> 0x18),
			      listptr->displ4);
	}
	if (immcount > 0)
	{
	    build_1hex_number((opcode_pt) immadr.offset, listptr->imm1);
	    if (immadr.data & RELBIT)
		listptr->relimm[0] = '>';
	}
	if (immcount > 1)
	    build_1hex_number((opcode_pt) immadr.offset >> 0x8,
			      listptr->imm2);
	if (immcount > 2)
	{
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x10),
			      listptr->imm3);
	    build_1hex_number((opcode_pt) (immadr.offset >> 0x18),
			      listptr->imm4);
	}
    }
    writes((char *) listptr);
}

/* list errors, assuming some */

PRIVATE void listerrors P0()
{
    unsigned char column;
    unsigned char errcol;	/* column # in error line */
    unsigned char errcolw;	/* working column in error line */
    _CONST char *errmsg;
    struct error_s *errptr;
    _CONST char *linep;
    unsigned char remaining;

    paderrorline(CODE_LIST_LENGTH - LINUM_LEN);
    remaining = errcount;
    column = 0;			/* column to match with error column */
    errcolw = errcol = CODE_LIST_LENGTH; /* working & col number on err line */
    errptr = errbuf;
    linep = linebuf;
    do
    {
	while (column < errptr->position)
	{
	    ++column;
	    if (*linep++ == '\t')	/* next tab (standard tabs only) */
		errcolw = (errcolw + 8) & 0xf8;
	    else
		++errcolw;
	    while (errcolw > errcol)
	    {
		writec(' ');
		++errcol;
	    }
	}
	if (errcolw < errcol)	/* position under error on new line */
	{
	    writenl();
	    paderrorline(errcolw - LINUM_LEN);
	}
	writec('^');
	writes(errmsg = build_error_message(errptr->errnum, heapptr));
	errcol += strlen(errmsg);
	++errptr;
    }
    while (--remaining != 0);
    writenl();
    if (erroverflow)
    {
	paderrorline(CODE_LIST_LENGTH - LINUM_LEN);
	writesn(build_error_message(FURTHER, heapptr));
    }
}

/* pad out error line to begin under 1st char of source listing */

PRIVATE void paderrorline P1(unsigned, nspaces)
{
    int nstars = LINUM_LEN;

    while (nstars-- != 0)
	writec('*');		/* stars under line number */
    while (nspaces-- != 0)
	writec(' ');		/* spaces out to error position */
}

/* write 1 character */

PUBLIC void writec P1(int, c)
{
    /* !! On a little-endian system, just do: (void)!write(innum, (char*) &c, 1); */
    char buf[1];

    buf[0] = c;
    (void)!write(innum, buf, 1);
}

/* write newline */

PUBLIC void writenl P0()
{
    writes("\n");  /* writec('\n'); */
}

/* write 1 offset_t, order to suit target */

PUBLIC void writeoff P1(offset_t, offset)
{
    char buf[4];

    u4c4(buf, (u4_pt) offset);
    (void)!write(innum, buf, 4);
}

/* write string */

PUBLIC void writes P1(_CONST char *, s)
{
    (void)!write(innum, s, strlen(s));
}

/* write string followed by newline */

PUBLIC void writesn P1(_CONST char *, s)
{
    writes(s);
    writenl();
}

/* write 1 word, order to suit target */

PUBLIC void writew P1(unsigned, word)
{
    char buf[2];

    u2c2(buf, (u2_pt) word);
    (void)!write(innum, buf, 2);
}
