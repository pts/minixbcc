/* io.c - input/output and error modules for linker */

#include <sys/types.h>
#include <sys/stat.h>
#define MY_STAT_H
#include <fcntl.h>
#include <unistd.h>
#include "const.h"
#include "obj.h"		/* needed for LONG_OFFSETS and offset_t */
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
PRIVATE char *inputname;	/* name of current input file */
PRIVATE char outbuf[OUTBUFSIZE];  /* output buffer */
PRIVATE char *outbufptr;	/* output buffer ptr */
PRIVATE char *outbuftop;	/* output buffer top */
PRIVATE int outfd;		/* output file descriptor */
PRIVATE unsigned outputperms;	/* permissions of output file */
PRIVATE char *outputname;	/* name of output file */
PRIVATE char *refname;		/* name of program for error reference */
PRIVATE unsigned warncount;	/* count of warnings */

FORWARD void flushout P((void));
FORWARD void outhexdigs P((offset_t num));
FORWARD void outputerror P((char *message));
FORWARD void put04x P((unsigned num));
FORWARD void putstrn P((char *message));

PUBLIC void ioinit(progname)
char *progname;
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

PUBLIC void closein()
{
    if (infd != ERR && close(infd) < 0)
	inputerror("cannot close");
    infd = ERR;
}

PUBLIC void closeout()
{
    flushout();
    if (close(outfd) == ERR)
	outputerror("cannot close");
}

PUBLIC void executable()
{
    int oldmask;

    if (errcount == 0)
    {
	oldmask = umask(0);
	umask(oldmask);
	chmod(outputname, outputperms | (EXEC_PERMS & ~oldmask));
    }
}

PUBLIC void flusherr()
{
    (void)!write(STDOUT_FILENO, errbuf, errbufptr - errbuf);
    errbufptr = errbuf;
}

PRIVATE void flushout()
{
    int nbytes;

    nbytes = outbufptr - outbuf;
    if (write(outfd, outbuf, nbytes) != nbytes)
	outputerror("cannot write");
    outbufptr = outbuf;
}

#ifdef OPEN00
  extern int open00(/* _CONST char *pathname */);  /* flags and mode are both 0. */
#else
#  define open00(pathname) open(pathname, 0  /* O_RDONLY */)
#endif

PUBLIC void openin(filename)
char *filename;
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

PUBLIC void openout(filename)
char *filename;
{
    struct stat statbuf;

    outputname = filename;
    if ((outfd = creat(filename, CREAT_PERMS)) == ERR)
	outputerror("cannot open");
    if (fstat(outfd, &statbuf) != 0) 
	outputerror("cannot stat");
    outputperms = statbuf.st_mode;
    chmod(filename, outputperms & ~EXEC_PERMS);
    outbufptr = outbuf;
}

PUBLIC char hexdigit[] = "0123456789abcdef";

PUBLIC void puthexdig(num)
unsigned num;
{
    putbyte(hexdigit[num & 0xf]);
}

PRIVATE void outhexdigs(num)
register offset_t num;
{
    if (num >= 0x10)
    {
	outhexdigs(num >> 4);
    }
    puthexdig((unsigned) num);
}

PRIVATE void put04x(num)
register unsigned num;
{
    puthexdig(num >> 12);
    puthexdig(num >> 8);
    puthexdig(num >> 4);
    puthexdig(num);
}

#ifdef LONG_OFFSETS

PUBLIC void put08lx(num)  /* Used by dumpsyms(). */
register offset_t num;
{
    put04x(num >> 16);
    put04x(num & 0xffff);
}

#else /* not LONG_OFFSETS */

PUBLIC void put08x(num)  /* Used by dumpsyms(). */
register offset_t num;
{
    putstr("0000");
    put04x(num);
}

#endif /* not LONG_OFFSETS */

PUBLIC void putbstr(width, str)
unsigned width;
char *str;
{
    unsigned length;
    
    for (length = strlen(str); length < width; ++length)
	putbyte(' ');
    putstr(str);
}

PUBLIC void putbyte(ch)
int ch;
{
    register char *ebuf;

    ebuf = errbufptr;
    if (ebuf >= errbuftop)
    {
	flusherr();
	ebuf = errbufptr;
    }
    *ebuf++ = ch;
    errbufptr = ebuf;
}

PUBLIC void putstr(message)
char *message;
{
    while (*message != 0)
	putbyte(*message++);
}

PRIVATE void putstrn(message)
char *message;
{
    putstr(message);
    putbyte('\n');
    flusherr();
}

PUBLIC int readchar()
{
    int ch;
	
    register char *ibuf;
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
    ch = (unsigned char) *ibuf++;
    inbufptr = ibuf;
    return ch;
}

PUBLIC void readin(buf, count)
char *buf;
unsigned count;
{
    int ch;
    
    while (count--)
    {
	if ((ch = readchar()) < 0)
	    prematureeof();
	*buf++ = ch;
    }
}

PUBLIC bool_pt readineofok(buf, count)
char *buf;
unsigned count;
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

PUBLIC void seekin(offset)
long offset;
{
    inbufptr = inbufend = inbuf;
    if (lseek(infd, offset, SEEK_SET) < 0)
	prematureeof();
}

PUBLIC void seekout(offset)
long offset;
{
    flushout();
    if (lseek(outfd, offset, SEEK_SET) != offset)
	outputerror("cannot seek in");
}

PUBLIC void writechar(ch)
int ch;
{
    register char *obuf;

    obuf = outbufptr;
    if (obuf >= outbuftop)
    {
	flushout();
	obuf = outbufptr;
    }
    *obuf++ = ch;
    outbufptr = obuf;
}

PUBLIC void writeout(buf, count)
register char *buf;
unsigned count;
{
    register char *obuf;

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

PUBLIC void errexit(message)
char *message;
{
    putstrn(message);
    exit(2);
}

PUBLIC void fatalerror(message)
char *message;
{
    refer();
    errexit(message);
}

PUBLIC void inputerror(message)
char *message;
{
    refer();
    putstr(message);
    putstr(" input file ");
    errexit(inputname);
}

PUBLIC void input1error(message)
char *message;
{
    refer();
    putstr(inputname);
    errexit(message);
}

PRIVATE void outputerror(message)
char *message;
{
    refer();
    putstr(message);
    putstr(" output file ");
    errexit(outputname);
}

PUBLIC void outofmemory()
{
    inputerror("out of memory while processing");
}

PUBLIC void prematureeof()
{
    inputerror("premature end of");
}

PUBLIC void redefined(name, message, archentry, deffilename, defarchentry)
char *name;
char *message;
char *archentry;
char *deffilename;
char *defarchentry;
{
    ++warncount;
    refer();
    putstr("warning: ");
    putstr(name);
    putstr(" redefined");
    putstr(message);
    putstr(" in file ");
    putstr(inputname);
    if (archentry != NULL)
    {
	putbyte('(');
	putstr(archentry);
	putbyte(')');
    }
    putstr("; using definition in ");
    putstr(deffilename);
    if (defarchentry != NULL)
    {
	putbyte('(');
	putstr(defarchentry);
	putbyte(')');
    }
    putbyte('\n');
}

PUBLIC void refer()
{
    putstr(refname);
    putstr(": ");
}

PUBLIC void reserved(name)
char *name;
{
    ++errcount;
    putstr("incorrect use of reserved symbol: ");
    putstrn(name);
}

PUBLIC void size_error(seg, count, size)
char seg;
offset_t count;
offset_t size;
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

PUBLIC void undefined(name)
char *name;
{
    ++errcount;
    putstr("undefined symbol: ");
    putstrn(name);
}

PUBLIC void usage()
{
    putstr("usage: ");
    putstr(refname);
    errexit(
    " [-03Mimsz[-]] [-T textaddr] [-llib_extension] [-o outfile] infile...");
}
