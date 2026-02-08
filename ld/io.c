/* io.c - input/output and error modules for linker */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#  include <fcntl.h>
#  include <unistd.h>
#  include <string.h>
#  include <stdlib.h>
#endif
#ifndef _AUTOC_H
#  include "autoc.h"  /* For INT32T. */
#endif
#include "const.h"
#include "obj.h"
#include "type.h"
#ifdef GLOBVARI
#  include "globvar"  /* Workaround for #include basename bug. `globvar' and `globvar.h' are the same. */
#else
#  include "globvar.h"
#endif

#ifndef INBUFSIZE  /* Configurable at compile time: -DINBUFSIZE=... */
#  ifdef SMALLMEM
#    define INBUFSIZE  1024
#  else
#    define INBUFSIZE  1024
#  endif
#endif
#ifndef OUTBUFSIZE  /* Configurable at compile time: -DOUTBUFSIZE=... */
#  ifdef SMALLMEM
#    define OUTBUFSIZE  1024
#  else
#    define OUTBUFSIZE  2048
#  endif
#endif
#ifndef ERRBUFSIZE  /* Configurable at compile time: -DERRBUFSIZE=... */
#  ifdef SMALLMEM
#    define ERRBUFSIZE  512
#  else
#    define ERRBUFSIZE  1024
#  endif
#endif

#define ERR		-1

PUBLIC unsigned errcount;		/* count of errors */
PRIVATE char errbuf[ERRBUFSIZE];  /* error buffer (actually uses STDOUT) */
PRIVATE char *errbufptr;	/* error buffer ptr */
PRIVATE char *errbuftop;	/* error buffer top */
PRIVATE char inbuf[INBUFSIZE];	/* input buffer */
PRIVATE char *inbufend;		/* input buffer top */
PRIVATE char *inbufptr;		/* end of input in input buffer */
PRIVATE int infd;		/* input file descriptor */
PRIVATE _CONST char *inputname;	/* name of current input file */
PRIVATE char outbuf[OUTBUFSIZE];  /* output buffer */
PRIVATE char *outbufptr;	/* output buffer ptr */
PRIVATE char *outbuftop;	/* output buffer top */
PRIVATE int outfd;		/* output file descriptor */
PRIVATE unsigned outputperms;	/* permissions of output file */
PRIVATE _CONST char *outputname;  /* name of output file */
PRIVATE _CONST char *refname;	/* name of program for error reference */
PRIVATE unsigned warncount;	/* count of warnings */

FORWARD void flushout P((void));
FORWARD void outhexdigs P((offset_t num));
FORWARD void outputerror P((_CONST char *message));
FORWARD void put04x P((unsigned num));
FORWARD void putstrn P((_CONST char *message));

PUBLIC void ioinit P1(_CONST char *, progname)
{
    infd = ERR;
    if (*progname)
	refname = progname;	/* name must be static (is argv[0]) */
    else
	refname = "link";
    errbufptr = errbuf;
    errbuftop = errbuf + ERRBUFSIZE;
    outbuftop = outbuf + OUTBUFSIZE;
}

PUBLIC void closein P0()
{
    if (infd != ERR && close(infd) < 0)
	inputerror("cannot close");
    infd = ERR;
}

PUBLIC void closeout P0()
{
    flushout();
    if (close(outfd) == ERR)
	outputerror("cannot close");
}

PUBLIC void executable P0()
{
    int oldmask;

    if (errcount == 0)
    {
	oldmask = umask(0);
	umask(oldmask);
	chmod(outputname, outputperms | (EXEC_PERMS & ~oldmask));
    }
}

PUBLIC void flusherr P0()
{
    (void)!write(STDOUT_FILENO, errbuf, errbufptr - errbuf);
    errbufptr = errbuf;
}

PRIVATE void flushout P0()
{
    int nbytes;

    nbytes = outbufptr - outbuf;
    if (write(outfd, outbuf, nbytes) != nbytes)
	outputerror("cannot write");
    outbufptr = outbuf;
}

#ifdef OPEN00
  extern int open00 P((_CONST char *pathname));  /* flags and mode are both 0. */
#else
#  define open00(pathname) open(pathname, 0  /* O_RDONLY */)
#endif

PUBLIC void openin P1(_CONST char *, filename)
{
    if (infd == ERR || strcmp(inputname, filename) != 0)
    {
	closein();
	inputname = filename;	/* this relies on filename being static */
	if ((infd = open00(filename)) < 0)
	    inputerror("cannot open");
	inbufptr = inbufend = inbuf;
    }
}

PUBLIC void openout P1(_CONST char *, filename)
{
#ifdef __WATCOMC__  /* OpenWatcom v2 or earlier. */
#  ifdef _WCDATA  /* OpenWatcom v2 libc. */
#    ifdef __LINUX__  /* owcc -blinux */
#      define STATBUFCOUNT 2  /* Workaround for `sh build.sh owcc', without the `-I"$WATCOM"/lh' or `export INCLUDE=$WATCOM/lh', and using (by default) -I"$WATCOM"/h (smaller struct_stat) instead of the correct -I"$WATCOM"/lh (larger struct_stat) on Linux. */
#    endif
#  endif
#endif
#ifndef STATBUFCOUNT
#  define STATBUFCOUNT 1
#endif
    struct stat statbufs[STATBUFCOUNT];

    outputname = filename;
    if ((outfd = creat(filename, CREAT_PERMS)) == ERR)
	outputerror("cannot open");
    if (fstat(outfd, statbufs) != 0)
	outputerror("cannot stat");
    outputperms = statbufs[0].st_mode;  /* .st_mode is at the same location within struct_stat in -I"$WATCOM"/h and -I"$WATCOM"/lh */
    chmod(filename, outputperms & ~EXEC_PERMS);
    outbufptr = outbuf;
}

PUBLIC char hexdigit[] = "0123456789abcdef";

PUBLIC void puthexdig P1(unsigned, num)
{
    putbyte(hexdigit[num & 0xf]);
}

PRIVATE void outhexdigs P1(REGISTER offset_t, num)
{
    if (num >= 0x10)
    {
	outhexdigs(num >> 4);
    }
    puthexdig((unsigned) num);
}

PRIVATE void put04x P1(REGISTER unsigned, num)
{
    puthexdig(num >> 12);
    puthexdig(num >> 8);
    puthexdig(num >> 4);
    puthexdig(num);
}

PUBLIC void put08lx P1(REGISTER offset_t, num)  /* Used by dumpsyms(). */
{
    put04x((unsigned) (num >> 16));
    put04x((unsigned) num & 0xffff);
}

PUBLIC void putbstr P2(unsigned, width, _CONST char *, str)
{
    unsigned length;
    
    for (length = strlen(str); length < width; ++length)
	putbyte(' ');
    putstr(str);
}

PUBLIC void putbyte P1(int, ch)
{
    REGISTER char *ebuf;

    ebuf = errbufptr;
    if (ebuf >= errbuftop)
    {
	flusherr();
	ebuf = errbufptr;
    }
    *ebuf++ = ch;
    errbufptr = ebuf;
}

PUBLIC void putstr P1(_CONST char *, message)
{
    while (*message != 0)
	putbyte(*message++);
}

PRIVATE void putstrn P1(_CONST char *, message)
{
    putstr(message);
    putbyte('\n');
    flusherr();
}

PUBLIC int readchar P0()
{
    int ch;
	
    REGISTER char *ibuf;
    int nread;

    ibuf = inbufptr;
    if (ibuf >= inbufend)
    {
	ibuf = inbufptr = inbuf;
	nread = read(infd, ibuf, INBUFSIZE);
	if (nread <= 0)
	{
	    inbufend = ibuf;
	    return ERR;
	}
	inbufend = ibuf + nread;
    }
#ifdef ACKFIX  /* For Minix 1.5.10 i86 ACK 3.1 C compiler. */
    ch = *ibuf++ & 0xff;  /* It works with any C compiler. */
#else
    ch = (unsigned char) *ibuf++;  /* The Minix 1.5.10 i86 ACK 3.1 C compiler is buggy: it ignores the `(unsigned char)' cast here. */
#endif
    inbufptr = ibuf;
    return ch;
}

PUBLIC void readin P2(char *, buf, unsigned, count)
{
    int ch;

    while (count--)
    {
	if ((ch = readchar()) < 0)
	    prematureeof();
	*buf++ = ch;
    }
}

PUBLIC bool_pt readineofok P2(char *, buf, unsigned, count)
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    return TRUE;
	*buf++ = ch;
    }
    return FALSE;
}

#if __STDC__
#  define cast_lseek_offset(offset) (offset)
#  define const_lseek_offset(v) ((INT32T) (v))  /* Without the cast, it would be broken with `cc -m' on Minix 2.0.4 i86, which runs `irrel -m', which strips arguments from function prototypes, thus it would push only 16 bits. */
#else
#  define cast_lseek_offset(offset) ((off_t) (offset))  /* We must pass the offset of the correct size, because the K&R C compiler doesn't know the argument type of lseek(...). Compile with -Doff_t=long if needed. */
#  define const_lseek_offset(v) ((off_t) (v))
#endif

PUBLIC void seekin P1(unsigned INT32T, offset)
{
    inbufptr = inbufend = inbuf;
    if ((INT32T) offset < 0 || lseek(infd, cast_lseek_offset(offset), SEEK_SET) < 0)
	prematureeof();
}

PUBLIC void seekout P1(unsigned INT32T, offset)
{
    flushout();
    if ((INT32T) offset < 0 || lseek(outfd, cast_lseek_offset(offset), SEEK_SET) != cast_lseek_offset(offset))
	outputerror("cannot seek in");
}

PUBLIC void writechar P1(int, ch)
{
    REGISTER char *obuf;

    obuf = outbufptr;
    if (obuf >= outbuftop)
    {
	flushout();
	obuf = outbufptr;
    }
    *obuf++ = ch;
    outbufptr = obuf;
}

PUBLIC void writeout P2(REGISTER _CONST char *, buf, unsigned, count)
{
    REGISTER char *obuf;

    obuf = outbufptr;
    while (count--)
    {
	if (obuf >= outbuftop)
	{
	    outbufptr = obuf;
	    flushout();
	    obuf = outbufptr;
	}
	*obuf++ = *buf++;
    }
    outbufptr = obuf;
}

/* error module */

PUBLIC void errexit P1(_CONST char *, message)
{
    putstrn(message);
    exit(2);
}

PUBLIC void fatalerror P1(_CONST char *, message)
{
    refer();
    errexit(message);
}

PUBLIC void inputerror P1(_CONST char *, message)
{
    refer();
    putstr(message);
    putstr(" input file ");
    errexit(inputname);
}

PUBLIC void input1error P1(_CONST char *, message)
{
    refer();
    putstr(inputname);
    errexit(message);
}

PRIVATE void outputerror P1(_CONST char *, message)
{
    refer();
    putstr(message);
    putstr(" output file ");
    errexit(outputname);
}

PUBLIC void outofmemory P0()
{
    inputerror("out of memory while processing");
}

PUBLIC void prematureeof P0()
{
    inputerror("premature end of");
}

PUBLIC void redefined P5(_CONST char *, name, _CONST char *, message, _CONST char *, archentry, _CONST char *, deffilename, _CONST char *, defarchentry)
{
    ++warncount;
    refer();
    putstr("warning: ");
    putstr(name);
    putstr(" redefined");
    putstr(message);
    putstr(" in file ");
    putstr(inputname);
    if (archentry != (char*) 0)
    {
	putbyte('(');
	putstr(archentry);
	putbyte(')');
    }
    putstr("; using definition in ");
    putstr(deffilename);
    if (defarchentry != (char*) 0)
    {
	putbyte('(');
	putstr(defarchentry);
	putbyte(')');
    }
    putbyte('\n');
}

PUBLIC void refer P0()
{
    putstr(refname);
    putstr(": ");
}

PUBLIC void reserved P1(_CONST char *, name)
{
    ++errcount;
    putstr("incorrect use of reserved symbol: ");
    putstrn(name);
}

PUBLIC void size_error P3(unsigned, seg, offset_t, count, offset_t, size)
{
    refer();
    putstr("seg ");
    outhexdigs((offset_t) seg);
    putstr(" has wrong size ");
    outhexdigs(count);
    putstr(", supposed to be ");
    outhexdigs(size);
    errexit("\n");
}

PUBLIC void undefined P1(_CONST char *, name)
{
    ++errcount;
    putstr("undefined symbol: ");
    putstrn(name);
}

PUBLIC void usage P0()
{
    putstr("usage: ");
    putstr(refname);
    errexit(
    " [-03Mimsz[-]] [-T textaddr] [-llib_extension] [-o outfile] infile...");
}
