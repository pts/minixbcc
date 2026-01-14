/* as.c - assembler */

/*
  usage: as source [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] [-guw]
  in any order (but no repeated file options)
*/

#include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "const.h"
#include "type.h"
#include "byteord.h"
#include "macro.h"
#undef EXTERN
#define EXTERN
#include "file.h"
#include "flag.h"
#include "globvar.h"

PRIVATE struct block_s hid_blockstak[MAXBLOCK];	/* block stack */
PRIVATE struct lc_s hid_lctab[NLOC];	/* location counter table */
PRIVATE struct if_s hid_ifstak[MAXBLOCK];	/* if stack */
PRIVATE struct schain_s hid_mcpar[MACPSIZ];	/* MACRO params */
PRIVATE struct macro_s hid_macstak[MAXBLOCK];	/* macro stack */
PRIVATE struct sym_s *hid_spt[SPTSIZ];	/* hash table */

FORWARD void initp1 P((void));
FORWARD int my_creat P((char *name, char *message));
FORWARD void process_args P((int argc, char **argv));
FORWARD void summary P((fd_t fd));
FORWARD void summ_number P((unsigned num));
FORWARD void usage P((void));

#ifdef MINIXHEAP
extern char *brksize;  /* Defined by libc. */
extern char *brk();  /* Defined by libc. */

PRIVATE int trybrk(p) char *p; {
  /* brk(...) makes sure that a minimmum amount of stack space is still available above the heap. */
  return !brk(p) && brksize >= p;  /* brk(...) also changes brksize. */
}
#endif

PRIVATE void initheap()
{
#ifdef MINIXHEAP
  unsigned a, b, m;

  /* Use binary search to find the maximum heap size available, and allocate it. */
  heapptr = brksize;
#ifdef DEBUG_MINIXHEAP
  (void)!write(2, "I", 1);
#endif
  for (a = 0, b = 1024; heapptr + b > heapptr && trybrk(heapptr + b); ) {
    a = b;
    b <<= 1;
#ifdef DEBUG_MINIXHEAP
    (void)!write(2, ",", 1);
#endif
  }
  if (!b) --b;  /* Decrease on power-of-2 overflow to 0. */
#ifdef DEBUG_MINIXHEAP
  (void)!write(2, "_", 1);
#endif
  if (!a) as_abort("cannot allocate memory");  /* Minimum is 1 KiB. */
  for (;;) {
    /* Now `a' bytes are available, and `b' bytes are not available, and `a < b'. */
    if ((m = a + ((b - a) >> 1)) == a) break;
#ifdef DEBUG_MINIXHEAP
    (void)!write(2, "?", 1);
#endif
    if (heapptr + m > heapptr && trybrk(heapptr + m)) {
#ifdef DEBUG_MINIXHEAP
      (void)!write(2, "A", 1);
#endif
      a = m;
    } else {
#ifdef DEBUG_MINIXHEAP
      (void)!write(2, "B", 1);
#endif
      b = m;
    }
  }
  heapend = heapptr + a;
#ifdef DEBUG_MINIXHEAP
  (void)!write(2, ".\n", 2);
#endif
#else
  char *p;
  unsigned size;

  size = sizeof(int) <= 2 ? (unsigned) 0xAC00 : (unsigned) 0x20000L;  /* !! Make this configurable at compile time. */
  if (!(p = malloc(size))) {  /* If `size' bytes are not available, try smaller amounts. */
    while (size > 1024 && !(p = malloc((size >>= 1, size += size >> 1)))) {}
    if (!p && size > 1024 && !(p = malloc(size = 1024))) {
      as_abort("cannot allocate memory");  /* Minimum is 1 KiB. */
    }
  }
  heapptr = p;
  heapend = p + size;
#endif
}

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    initheap();
    initp1();
    initp1p2();
    inst_keywords();
    initbin();
    initobj();
    initsource();		/* only nec to init for unsupported mem file */
    typeconv_init(AS_BIG_ENDIAN, LONG_BIG_ENDIAN);
    warn.global = TRUE;		/* constant */
    process_args(argc, argv);
    initscan();

    assemble();			/* doesn't return, maybe use setjmp */

    /* NOTREACHED */
    return 0;
}

PUBLIC void as_abort(message)
char *message;
{
    (void)!write(STDOUT, "as: ", 4);
    (void)!write(STDOUT, message, strlen(message));
    (void)!write(STDOUT, "\n", 1);
    exit(1);
}

PUBLIC void finishup()
{
    bintrailer();
    objtrailer();
    if (list.global ||symgen)
	gensym();		/* output to lstfil and/or symfil */
    if (list.global ||toterr != 0 || totwarn != 0)
	summary(lstfil);
    if (lstfil != STDOUT && (toterr != 0 || totwarn != 0))
	summary(STDOUT);
    statistics();
    exit(toterr != 0 ? 1 : 0);	/* should close output files and check */
}

/* initialise constant nonzero values */

PRIVATE void initp1()
{
#ifdef I80386
    idefsize = defsize = sizeof (char *) > 2 ? 4 : 2;
#endif
    lctabtop = lctab + NLOC;
    lstfil = STDOUT;
    mapnum = 15;		/* default map number for symbol table */
    spt_top = (spt = hid_spt) + SPTSIZ;
}

/* initialise nonzero values which start same each pass */

PUBLIC void initp1p2()
{
    register struct lc_s *lcp;

    ifflag = TRUE;
    pedata = UNDBIT;		/* program entry point not defined */
    blockstak = hid_blockstak + MAXBLOCK;
    ifstak = hid_ifstak + MAXIF;
    macstak = hid_macstak + MAXMAC;
    macptop = (macpar = hid_mcpar) + MACPSIZ;
    lctabtop = (lcptr = lctab = hid_lctab) + NLOC;
    for (lcp = lctab; lcp < lctabtop; ++lcp)
	/* init of lcdata/lc (many times) in loop to save space */
    {
	lcp->data = lcdata = RELBIT;	/* lc relocatable until 1st ORG */
	lcp->lc = lc = 0;
    }
}

PRIVATE int my_creat(name, message)
char *name;
char *message;
{
    int fd;

    if ((fd = creat(name, CREAT_PERMS)) < 0 || fd > 255)
	as_abort(message);
    return fd;
}

PRIVATE void process_args(argc, argv)
int argc;
char **argv;
{
    char *arg;
    bool_t isnextarg;
    char *nextarg;

    if (argc <= 1)
	usage();
    do
    {
	arg = *++argv;
	if (arg[0] == '-')
	{
	    if (arg[2] != 0)
		usage();	/* no multiple options */
	    isnextarg = FALSE;
	    if (argc > 2)
	    {
		nextarg = argv[1];
		if (nextarg[0] != 0 && nextarg[0] != '-')
		    isnextarg = TRUE;
	    }
	    switch (arg[1])
	    {
#ifdef I80386
	    case '0':
		idefsize = defsize = 0x2;
		break;
	    case '3':
		idefsize = defsize = 0x4;
		break;
	    case 'a':
		asld_compatible = TRUE;
		break;
#endif
	    case 'b':
		if (!isnextarg || binfil != 0)
		    usage();
		binfil = my_creat(nextarg, "error creating binary file");
		binaryg = TRUE;
		--argc;
		++argv;
		break;
	    case 'g':
		globals_only_in_obj = TRUE;
		break;
#ifdef I80386
	    case 'j':
		jumps_long = TRUE;
		break;
#endif
	    case 'l':
		list.global = TRUE;
		goto get_any_list_file;
	    case 'm':
		maclist.global = TRUE;
	get_any_list_file:
		if (isnextarg)
		{
		    if (lstfil != STDOUT)
			usage();
		    lstfil = my_creat(nextarg, "error creating list file");
		    --argc;
		    ++argv;
		}
		break;
	    case 'n':
		if (!isnextarg)
		    usage();
		truefilename = nextarg;
		--argc;
		++argv;
		break;
	    case 'o':
		if (!isnextarg || objfil != 0)
		    usage();
		objectg = TRUE;
		objfil = my_creat(nextarg, "error creating object file");
		--argc;
		++argv;
		break;
	    case 's':
		if (!isnextarg || symfil != 0)
		    usage();
		symgen = TRUE;
		symfil = my_creat(nextarg, "error creating symbol file");
		--argc;
		++argv;
		break;
	    case 'u':
		inidata = IMPBIT | SEGM;
		break;
	    case 'w':
		warn.semaphore = -1;
		break;
	    default:
		usage();	/* bad option */
	    }
	}
	else if (infil != 0)
	    usage();		/* no multiple sources */
	else
	{
	    if (strlen(arg) > FILNAMLEN)
		as_abort("source file name too long");
	    infil = open_input(strcpy(filnamptr, arg));
	    infiln = infil0 = 1;
	}
    }
    while (--argc != 1);
    inidata = (~binaryg & inidata) | (RELBIT | UNDBIT);
}				/* IMPBIT from inidata unless binaryg */

PRIVATE void summary(fd)
int fd;
{
    innum = fd;
    writenl();
    summ_number(toterr);
    writesn(" errors");
    summ_number(totwarn);
    writesn(" warnings");
}

PRIVATE void summ_number(num)
unsigned num;
{
    /* format number like line numbers, build it at free spot heapptr */
    *build_number(num, LINUM_LEN, heapptr) = 0;
    writes(heapptr);
}

PRIVATE void usage()
{
    as_abort(
#ifdef I80386
"usage: as [-03agjuw] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
#else
    "usage: as [-guw] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
#endif
}
