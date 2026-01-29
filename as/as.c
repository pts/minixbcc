/* as.c - assembler */

/*
  usage: as source [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] [-guw]
  in any order (but no repeated file options)
*/

#ifdef LIBCH
#  include "libc.h"
#else
#  include <sys/types.h>  /* Minix 1.5.10 needs this before <unistd.h>. */
#  include <fcntl.h>
#  include <stdlib.h>
#  include <unistd.h>
#  include <string.h>
#endif
#include "const.h"
#include "type.h"
#include "macro.h"
#include "scan.h"
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

FORWARD void set_osid P((int new_osid));
FORWARD void initp1 P((void));
FORWARD int my_creat P((char *name, char *message));
FORWARD void process_args P((int argc, char **argv));
FORWARD void summary P((fd_t fd));
FORWARD void summ_number P((unsigned num));
FORWARD void usage P((void));
FORWARD void set_label_abs P((char *name, offset_t value));
FORWARD void initblabels P((void));

#ifdef DEBUG_AS
#  define D4(msg) (void)!write(2, msg, 4);
#else
#  define D4(msg)
#endif

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    D4("M01\n")
    initheap();
    D4("M02\n")
    initp1();
    D4("M03\n")
    initp1p2();
    D4("M04\n")
    inst_keywords();
    D4("M05\n")
    initbin();
    D4("M06\n")
    initobj();
    D4("M07\n")
    initsource();		/* only nec to init for unsupported mem file */
    D4("M08\n")
    initblabels();  /* Must be called before process_args() to create symbols. -- why?. */
    D4("M09\n")
    typeconv_init();
    D4("M10\n")
    warn.global = TRUE;		/* constant */
    D4("M11\n")
    process_args(argc, argv);
    D4("M12\n")
    initscan();
    D4("M13\n")
    initblabels();  /* Call agan to set the final values of the symbols. */

    D4("M14\n")
    assemble();			/* doesn't return, maybe use setjmp */
    D4("M15\n")

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

PRIVATE void set_osid(new_osid)
register int new_osid;
{
	osid = new_osid;
	idefsize = defsize = 2 + ((new_osid & 1) << 1);  /* 2 if osid == 0; or 4 if osid == 3. */
}

PRIVATE void set_label_abs(name, value)
char *name;
offset_t value;
{
    register struct sym_s *symptr;
    char *old_lineptr;
    /* char *old_symname; */

    old_lineptr = lineptr;
    /* old_symname = symname; */
    lineptr = (symname = name) + strlen(name);  /* Symbol name for lookup() below. */
    (symptr = lookup())->type = LABIT | VARBIT;  /* Adding VARBIT here so that objheader(...) will omit it from the symbol list. */
    /* symptr->data = 0; */  /* Default: absolute value. */
    symptr->value_reg_or_op.value = value;
    /* symname = old_symname; */  /* No need to restore. */
    lineptr = old_lineptr;  /* Must be restored. */
}

/* initialise builtin labels (absolute symbols with constant value) */

PRIVATE void initblabels()
{
    set_label_abs("__IBITS__", (offset_t) (idefsize << 3));  /* 16 or 32. This is the initial bits, not the current one. */
    set_label_abs("__OSID__", (offset_t) osid);  /* 0 (for as -0: Minix i86) or 3 (for as -3: Minix i386). */
}

/* initialise constant nonzero values */

PRIVATE void initp1()
{
    set_osid(sizeof(char *) > 2 ? 3 : 0);
    lstfil = STDOUT;
    mapnum = 15;		/* default map number for symbol table */
    spt_top = (spt = hid_spt) + SPTSIZ;
    binfil = -1;
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

    if ((fd = creat(name, CREAT_PERMS)) < 0 || fd > 255)  /* !! Why can't it be larger than 255? Is it saved somewhere? */
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

    nextarg = (char *) 0;  /* Pacify GCC warning -Wmaybe-uninitialized. */
    if (argc <= 1)
	usage();
    do
    {
	arg = *++argv;
#ifdef DEBUG_AS
	(void)!write(2, "PARGV0=(", 8); (void)!write(2, arg, strlen(arg)); D4(")..\n");
	(void)!write(2, "PARGV1=(", 8); (void)!write(2, argv[1], strlen(argv[1])); D4(")..\n");
#endif
	if (arg[0] == '-')
	{
	    D4("P02\n")
	    if (arg[2] != 0)
		usage();	/* no multiple options */
	    D4("P03\n")
	    isnextarg = FALSE;
	    if (argc > 2)
	    {
		nextarg = argv[1];
		if (nextarg[0] != 0 && nextarg[0] != '-')
		    isnextarg = TRUE;
	    }
	    D4("P04\n")
#ifdef DEBUG_AS
	    (void)!write(2, "PARGV1=(", 8); (void)!write(2, argv[1], strlen(argv[1])); D4(")..\n");
#endif
	    switch (arg[1])
	    {
	    case '0':
	    case '3':
		D4("P05\n")
#ifdef DEBUG_AS
		(void)!write(2, "PARGV1=(", 8); (void)!write(2, argv[1], strlen(argv[1])); D4(")..\n");
#endif
		set_osid(arg[1] - '0');  /* osid 0 if arg[1] == '0'; or 3 if arg[1] == '3'. */
#ifdef DEBUG_AS
		(void)!write(2, "PARGV1=(", 8); (void)!write(2, argv[1], strlen(argv[1])); D4(")..\n");
#endif
		break;
	    case 'a':
		D4("P06\n")
		asld_compatible = TRUE;
		break;
	    case 'b':
		D4("P07\n")
		if (!isnextarg || binfil != -1)
		    usage();
		binfil = my_creat(nextarg, "error creating binary file");
		binaryg = TRUE;
		--argc;
		++argv;
		break;
	    case 'g':
		globals_only_in_obj = TRUE;
		break;
	    case 'j':
		jumps_long = TRUE;
		break;
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
	D4("P98\n")
#ifdef DEBUG_AS
	(void)!write(2, "PARGV1=(", 8); (void)!write(2, argv[1], strlen(argv[1])); D4(")..\n");
#endif
    }
    while (--argc != 1);
    D4("P99\n")
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
    as_abort("usage: as [-03agjuw] [-b [bin]] [-lm [list]] [-n name] [-o obj] [-s sym] src");
}
