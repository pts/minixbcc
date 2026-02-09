/* output.c - output and error handling for bcc */

/* Copyright (C) 1992 Bruce Evans */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#  include <fcntl.h>
#  include <stdlib.h>
#  include <unistd.h>
#endif
#include "const.h"
#include "types.h"
#include "input.h"
#include "os.h"
#include "sizes.h"
#include "table.h"

#undef EXTERN
#define EXTERN
#include "output.h"

#define HEXSTARTCHAR '$'
#define OUTBUFSIZE 2048
#define opcodeleadin()		/* outtab() for fussy assemblers */

PRIVATE unsigned errcount;	/* # errors in compilation */
				/* depends on zero init */
PRIVATE char hexdigits[] = "0123456789ABCDEF";
PRIVATE char *outbuf;
EXTERN char *outbufend;		/* end of pair of output buffers */
PRIVATE char *outbufmid;
PRIVATE fd_t output;
PRIVATE fastin_t outstage;	/* depends on zero init */

FORWARD void errorsummary P((void));
FORWARD void errsum1 P((void));

PUBLIC void bugerror P1(_CONST char *, message)
{
    error2error("compiler bug - ", message);
}

PUBLIC void closeout P0()
{
    char *saveoutptr;

    if (outstage == 3)
    {
	saveoutptr = outbufptr;
	flushout();		/* buffer from last stage */
	outbufptr = saveoutptr;
    }
    outstage = 0;		/* flush from top to current ptr */
    flushout();
    close(output);
}

/* error handler */

PUBLIC void error P1(_CONST char *, message)
{
    error2error(message, "");
}

/* error handler - concatenate 2 messages */

PUBLIC void error2error P2(_CONST char *, message1, _CONST char *, message2)
{
    _CONST char *warning;

    if (message1[0] == '%' && message1[1] == 'w')
    {
	message1 += 2;
	warning = "warning: ";
    }
    else
    {
	++errcount;
	warning = "error: ";
    }
    if (output != 1)
    {
	char *old_outbuf;
	char *old_outbufptr;
	char *old_outbuftop;
	fd_t old_output;
	fastin_t old_outstage;
	char smallbuf[81];	/* don't use heap - might be full or busy */

	old_outbuf = outbuf;
	old_outbufptr = outbufptr;
	old_outbuftop = outbuftop;
	old_output = output;
	old_outstage = outstage;

	outbufptr = outbuf = &smallbuf[0];
	outbuftop = &smallbuf[sizeof smallbuf];
	output = 1;
	outstage = 0;
	errorloc();
	outstr(warning);
	outstr(message1);
	outstr(message2);
	outnl();
	flushout();

	outbuf = old_outbuf;
	outbufptr = old_outbufptr;
	outbuftop = old_outbuftop;
	output = old_output;
	outstage = old_outstage;
    }
    comment();
    errorloc();
    outstr(warning);
    outstr(message1);
    outstr(message2);
    outnl();
}

/* summarise errors */

PRIVATE void errorsummary P0()
{
    if (errcount != 0)
    {
	outfail();
	errsum1();
    }
    outnl();
    comment();
    errsum1();
}

PRIVATE void errsum1 P0()
{
    outudec(errcount);
    outnstr(" errors detected");
}

/* fatal error, exit early */

PUBLIC void fatalerror P1(_CONST char *, message)
{
    error(message);
    finishup();
}

/* finish up compile */

PUBLIC void finishup P0()
{
    if (!cppmode)
    {
	if (watchlc)
	{
	    cseg();
	    outop0str("if *-.program.start-");
	    outnhex(getlc());
	    outfail();
	    outnstr("phase error");
	    outop0str("endif\n");
	}
#ifdef HOLDSTRINGS
	dumpstrings();
#endif
	dumpglbs();
	errorsummary();
    }
    closein();
    closeout();
    exit(errcount == 0 ? 0 : 1);
}

/* flush output file */

PUBLIC void flushout P0()
{
    unsigned nbytes;

    switch (outstage)
    {
    case 0:
	nbytes = (unsigned) (outbufptr - outbuf);
	outbufptr = outbuf;
	break;
    case 2:
	nbytes = OUTBUFSIZE;
	outbufptr = outbuf;
	outbuftop = outbufmid;
	outstage = 3;
	break;
    default:
	nbytes = OUTBUFSIZE;
	if (outstage == 1)
	    nbytes = 0;
	outbufptr = outbufmid;
	outbuftop = outbufend;
	outstage = 2;
	break;
    }
    if (nbytes != 0)
    {
	if (!orig_cppmode)
	    clearlabels(outbufptr, outbufptr + nbytes);
	if ((unsigned) write(output, outbufptr, nbytes) != nbytes)
	{
	    (void)!write(2, "output file error\n", 18);
	    closein();
	    close(output);
	    exit(1);
	}
    }
}

PUBLIC void initout P0()
{
    static char smallbuf[1];

    outbufend = outbuftop = (outbuf = outbufptr = smallbuf) + sizeof smallbuf;
    output = 1;			/* standard output */
}

PUBLIC void limiterror P1(_CONST char *, message)
{
    error2error("compiler limit exceeded - ", message);
    finishup();
}

PUBLIC void openout P1(_CONST char *, oname)
{
    if (output != 1)
	fatalerror("more than one output file");
    if ((output = creat(oname, CREATPERMS)) < 0)
    {
	output = 1;
	fatalerror("cannot open output");
    }
}

/* print character */

PUBLIC void outbyte P1(int, c)
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    REGISTER char *outp;

    outp = outbufptr;
    *outp++ = c;
    outbufptr = outp;
    if (outp >= outbuftop)
	flushout();
#else /* !C_CODE etc */
#include "oasm1.h"  /* Move #asm and #endasm directives to a separate file to pacify the ACK ANSI C compiler 1.202 warning: asm: unknown control. */
#endif /* C_CODE etc */
}

/* print comma */

PUBLIC void outcomma P0()
{
    outbyte(',');
}

/* print line number in format ("# %u \"%s\"%s", nr, fname, str) */

PUBLIC void outcpplinenumber P3(unsigned, nr, _CONST char *, fname, _CONST char *, str)
{
    outstr("# ");
    outudec(nr);
    outstr(" \"");
    outstr(fname);
    outstr("\"");
    outnstr(str);
}

/* print unsigned offset, hex format */

PUBLIC void outhex P1(uoffset_t, num)
{
#ifdef HEXSTARTCHAR
    if (num >= 10)
	outbyte(HEXSTARTCHAR);
#endif
    outhexdigs(num);
#ifdef HEXENDCHAR
    if (num >= 10)
	outbyte(HEXENDCHAR);
#endif
}

/* print unsigned offset, hex format with digits only (no hex designator) */

PUBLIC void outhexdigs P1(REGISTER uoffset_t, num)
{
    /* !! Optimize this for size, non-recursive. */
    if (num >= 0x10)
    {
	outhexdigs(num >> 4);
	num &= 0xf;
    }
    outbyte(hexdigits[(unsigned) num]);
}

/* print string terminated by EOL */

PUBLIC void outline P1(_CONST char *, s)
{
    REGISTER char *outp;
    REGISTER _CONST char *rs;

    outp = outbufptr;
    rs = s;
#ifdef COEOL
    if (*rs == COEOL)
	++rs;
#endif
    do
    {
	*outp++ = *rs;
	if (outp >= outbuftop)
	{
	    outbufptr = outp;
	    flushout();
	    outp = outbufptr;
	}
    }
    while (*rs++ != EOL);
    outbufptr = outp;
#ifdef COEOL
    if (*rs == COEOL)
	outbyte(COEOL);
#endif
}

/* print minus sign */

PUBLIC void outminus P0()
{
    outbyte('-');
}

/* print character, then newline */

PUBLIC void outnbyte P1(int, byte)
{
    outbyte(byte);
    outnl();
}

/* print unsigned offset, hex format, then newline */

PUBLIC void outnhex P1(uoffset_t, num)
{
    outhex(num);
    outnl();
}

/* print newline */

PUBLIC void outnl P0()
{
    if (watchlc)
    {
	outtab();
	comment();
	outhex(getlc());
    }
    outbyte('\n');
}

/* print opcode and newline, bump lc by 1 */

PUBLIC void outnop1str P1(_CONST char *, s)
{
    opcodeleadin();
    outstr(s);
    bumplc();
    outnl();
}

/* print opcode and newline, bump lc by 2 */

PUBLIC void outnop2str P1(_CONST char *, s)
{
    opcodeleadin();
    outstr(s);
    bumplc2();
    outnl();
}

/* print string, then newline */

PUBLIC void outnstr P1(_CONST char *, s)
{
    outstr(s);
    outnl();
}

/* print opcode */

PUBLIC void outop0str P1(_CONST char *, s)
{
    opcodeleadin();
    outstr(s);
}

/* print opcode, bump lc by 1 */

PUBLIC void outop1str P1(_CONST char *, s)
{
    opcodeleadin();
    outstr(s);
    bumplc();
}

/* print opcode, bump lc by 2 */

PUBLIC void outop2str P1(_CONST char *, s)
{
    opcodeleadin();
    outstr(s);
    bumplc2();
}

/* print opcode, bump lc by 3 */

PUBLIC void outop3str P1(_CONST char *, s)
{
    opcodeleadin();
    outstr(s);
    bumplc3();
}

/* print plus sign */

PUBLIC void outplus P0()
{
    outbyte('+');
}

/* print signed offset, hex format */

PUBLIC void outshex P1(offset_t, num)
{
    if ((uvalue_t) num >= -(maxoffsetto + 1))
    {
	outminus();
#ifdef ACKFIX  /* For Minix 1.5.10 i86 ACK 3.1 C compiler, no matter the optimization setting (cc -O). */  /* Fix not needed when compiling this BCC sc by this BCC sc compiled with ACK. */
	num = (offset_t) (~(uoffset_t) num + 1);  /* It works with any C compiler doing 2s complement arithmetic. */
#else
	num = (offset_t) -(uoffset_t) num;  /* The Minix 1.5.10 i86 ACK 3.1 C compiler is buggy: it only negates the low 16 bits of the 32-bit variable here. */
#endif
    }
    outhex((uoffset_t) num);
}

/* print string  */

PUBLIC void outstr P1(_CONST char *, s)
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    REGISTER char *outp;
    REGISTER _CONST char *rs;

    outp = outbufptr;
    rs = s;
    while (*rs)
    {
	*outp++ = *rs++;
	if (outp >= outbuftop)
	{
	    outbufptr = outp;
	    flushout();
	    outp = outbufptr;
	}
    }
    outbufptr = outp;
#else /* !C_CODE etc */
#include "oasm2.h"  /* Move #asm and #endasm directives to a separate file to pacify the ACK ANSI C compiler 1.202 warning: asm: unknown control. */
#endif /* C_CODE etc */
}

/* print tab */

PUBLIC void outtab P0()
{
    outbyte('\t');
}

/* print unsigned, decimal format */

PUBLIC void outudec P1(unsigned, num)
{
    char str[10 + 1];

    str[sizeof str - 1] = 0;
    outstr(pushudec(str + sizeof str - 1, num));
}

/* push decimal digits of an unsigned onto a stack of chars */

PUBLIC char *pushudec P2(REGISTER char *, s, REGISTER unsigned, num)
{
    REGISTER unsigned reduction;

    while (num >= 10)
    {
	reduction = num / 10;
	*--s = num - 10 * reduction + '0';
	num = reduction;
    }
    *--s = num + '0';
    return s;
}

PUBLIC void setoutbufs P0()
{
    if (!isatty(output))  /* The return of isatty(...) here affects the jump instruction sizes generated by deflabel(...). For isatty(output) == 1, none of the jumps are short (with a 1-byte delta).  */
    {
	outbufptr = outbuf = (char*) ourmalloc(2 * OUTBUFSIZE);
#ifdef TS
ts_s_outputbuf += 2 * OUTBUFSIZE;
#endif
	outbufmid = outbuftop = outbufptr + OUTBUFSIZE;
	outbufend = outbufmid + OUTBUFSIZE;
	outstage = 1;
    }
    if (watchlc)
	outnstr(".program.start:\n");	/* kludge temp label */
}
