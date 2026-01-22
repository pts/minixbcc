/* cc.c - driver for minixbcc (v3) BCC (Bruce's C compiler) */

#define _POSIX_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#define FALSE	0
#define FORWARD	static
#ifndef NULL
#define NULL	0
#endif
#define PRIVATE	static
#define PUBLIC
#define TRUE	1

#if __STDC__
#define P(x)	x
#else
#define P(x)	()
#endif

typedef unsigned char bool_t;	/* boolean: TRUE if nonzero */

#define LIBDIR "/usr/minixbcc"
#define INCLUDE "include"

#define SC	"sc"
#define AS	"as"
#define LD	"ld"

#define CRTSO	"crtso.o"
#define LIBCA	"libc.a"

#define TMPNAME	"/tmp/bccYYYYXXXX"

#define ALLOC_UNIT	16	/* allocation unit for arg arrays */
#define DIRCHAR	'/'

struct arg_s
{
    int argc;
    char **argv;
    unsigned size;
};

PRIVATE struct arg_s asargs;	/* = NULL */
PRIVATE struct arg_s ccargs;	/* = NULL */
PRIVATE struct arg_s ldargs;	/* = NULL */
PRIVATE char *progname;
PRIVATE struct arg_s tmpargs;	/* = NULL */
PRIVATE bool_t verbose;		/* = FALSE */

#define IS_HOST_BITS32 (sizeof(char *) >= 4)

PRIVATE struct
{
    char libdir[sizeof(LIBDIR)];  /* The trailing '\0' will be replaced with '/'. */
    char host;  /* '0' or '3' */
    char slash;  /* '/' */
    char tool[3];
} path_tool = { LIBDIR, IS_HOST_BITS32 ? '3' : '0', '/', "??" };
typedef char assert_path_tool_size[sizeof(path_tool.tool) >= sizeof(SC) && sizeof(path_tool.tool) >= sizeof(AS) && sizeof(path_tool.tool) >= sizeof(LD) ? 1 : -1];

PRIVATE struct
{
    char libdir[sizeof(LIBDIR)];  /* The trailing '\0' will be replaced with '/'. */
    char target;  /* '?'. Will be replaced with '0' or '3' */
    char slash;  /* '/' */
    char crtso[sizeof(CRTSO)];
} path_crtso = { LIBDIR, '?', '/', CRTSO };

PRIVATE struct
{
    char libdir[sizeof(LIBDIR)];  /* The trailing '\0' will be replaced with '/'. */
    char target;  /* '?'. Will be replaced with '0' or '3' */
    char slash;  /* '/' */
    char crtso[sizeof(LIBCA)];
} path_libca = { LIBDIR, '?', '/', LIBCA };

/* We need this workaround since many old C compilers, including BCC con't support string literal concatenation in:
 * char path_include[] = "-I" LIBDIR INCLUDE;
 */
PRIVATE struct
{
    char flag[2];
    char libdir[sizeof(LIBDIR)];  /* The trailing '\0' will be replaced with '/'. */
    char include[sizeof(INCLUDE)];
} path_include = { {'-', 'I'}, LIBDIR, INCLUDE };

PRIVATE char bits32_arg[3] = "-?";  /*The '?' will be replaced with '0' or '3'. */

/* Who can say if the standard headers declared these? */
int chmod P((const char *name, int mode));
int execv P((char *name, char **argv));
void exit P((int status));
int fork P((void));
int getpid P((void));
void *malloc P((unsigned));
char *mktemp P((void));
void *realloc P((void *ptr, unsigned size));
void (*signal P((int sig, void (*func) P((int sig))))) P((int sig));
char *strcpy P((char *target, const char *source));
size_t strlen P((const char *s));
char *strrchr P((const char *s, int c));
int unlink P((const char *name));
int wait P((int *status));
int write P((int fd, char *buf, unsigned int nbytes));

FORWARD void addarg P((register struct arg_s *argp, char *arg));
FORWARD void fatal P((char *message));
FORWARD void killtemps P((void));
FORWARD void *my_malloc P((unsigned size, char *where));
FORWARD char *my_mktemp P((void));
FORWARD void my_unlink P((char *name));
FORWARD void outofmemory P((char *where));
FORWARD int run P((char *prog, char *arg1, char *arg2, struct arg_s *argp));
FORWARD void set_trap P((void));
FORWARD void show_who P((char *message));
FORWARD void startarg P((struct arg_s *argp));
FORWARD char *stralloc P((char *s));
FORWARD void trap P((int signum));
FORWARD void unsupported P((char *option, char *message));
FORWARD void writen P((void));
FORWARD void writes P((char *s));
FORWARD void writesn P((char *s));

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    register char *arg;
    int argcount = argc;
    bool_t *argdone = my_malloc((unsigned) argc * sizeof *argdone, "argdone");
    bool_t as_only = FALSE;
    char *basename;
    bool_t bits32 = IS_HOST_BITS32;
    bool_t cc_only = FALSE;
    bool_t debug = FALSE;
    bool_t echo = FALSE;
    unsigned errcount = 0;
    char ext;
    char *f_out = "a.out";
    int length;
    unsigned ncsfiles = 0;
    unsigned nfilters;
    unsigned nifiles = 0;
    unsigned nofiles = 0;
    char *o_out;
    bool_t prep_debug = FALSE;
    bool_t prep_only = FALSE;
    char *s_out;
    int status = 0;
    char *argval;

    path_tool.libdir[sizeof(path_tool.libdir) - 1] =
        path_crtso.libdir[sizeof(path_crtso.libdir) - 1] =
        path_libca.libdir[sizeof(path_libca.libdir) - 1] =
        path_include.libdir[sizeof(path_include.libdir) - 1] = '/';

    progname = argv[0];
    addarg(&asargs, "-u");
    addarg(&asargs, "-w");
    addarg(&ldargs, "-i");

    /* pass 1 over argv to gather compile options */
    for (; --argc != 0;)
    {
	arg = *++argv;
	*++argdone = TRUE;
	if (arg[0] == '-' && arg[1] != 0 && arg[2] == 0)
	    switch (arg[1])
	    {
	    case '0':
		bits32 = FALSE;
		break;
	    case '3':
		bits32 = TRUE;
		break;
	    case 'E':
		prep_debug = TRUE;
		++errcount;
		unsupported(arg, "preprocess");
		break;
	    case 'P':
		prep_only = TRUE;
		++errcount;
		unsupported(arg, "preprocess");
		break;
	    case 'O':
		/* unsupported( arg, "optimize" ); */  /* Ignore, BCC doesn't support optimizatoin. */
		break;
	    case 'S':
		cc_only = TRUE;
		break;
	    case 'V':
		echo = TRUE;
		break;
	    case 'c':
		as_only = TRUE;
		break;
	    case 'f':
		++errcount;
		unsupported(arg, "float emulation");
		break;
	    case 'g':
		debug = TRUE;	/* unsupported( arg, "debug" ); */
		break;
	    case 'o':
		if (--argc < 1)
		{
		    ++errcount;
		    show_who("output file missing after -o\n");
		}
		else
		{
		    ++nofiles;
		    f_out = *++argv;
		    *++argdone = TRUE;
		}
		break;
	    case 'p':
		++errcount;
		unsupported(arg, "profile");
		break;
	    case 'v':
		verbose = TRUE;
		break;
	    case 'M':		/* ld: print symbols linked */
	    case 'i':		/* ld: separate I & D output */
	    case 'm':		/* ld: print modules linked */
	    case 's':		/* ld: strip symbols */
	    case 'z':		/* ld: unmapped zero page */
		addarg(&ldargs, arg);
		break;
	    case 'D':
	    case 'U':
	    case 'I':
	    missing_parameter:
		++errcount;
		show_who("missing value for ");
		writen(arg);
		break;
	    case 'A':
	    case 'B':
	    case 'C':
	    case 'L':
	    case 't':
	    case 'T':
	    case 'h':
		if (argc == 1) goto missing_parameter;
		--argc;
		argval = *++argv;
		*++argdone = TRUE;
		goto handle_argval;
	    default:
		*argdone = FALSE;
		break;
	    }
	else if (arg[0] == '-')
	{
	    if (arg[1] == 0)
	    {
		++errcount;
		show_who("stdin (-) not supported\n");
		continue;
	    }
	    argval = arg + 2;
	  handle_argval:
	    switch (arg[1])
	    {
	    case 'A':
		addarg(&asargs, argval);
		break;
	    case 'B':
		++errcount;
		unsupported(arg, "substituted cc");
		break;
	    case 'C':
		addarg(&ccargs, argval);
		break;
	    case 'D':
	    case 'U':
	    case 'I':
		addarg(&ccargs, arg);  /* !! Rename to scargs. */
		break;
	    case 'L':
		/*addarg(&ldargs, argval);*/  /* !! Implement this in this file. */  /* !! Add support for -l and -L, expand -l and -L, path absolute library name to ld */
		unsupported(arg, "library path");
		break;
	    case 't':
		++errcount;
		unsupported(arg, "pass number");
		break;
	    case 'T':		/* ld: text base address; default: 0 */
		addarg(&ldargs, "-T");
	    add_ldarg:
		addarg(&ldargs, argval);
		break;
	    case 'h':		/* ld: dynamic memory size (including heap, stack, argv and environ); the default of 0 means automatic */
		addarg(&ldargs, "-h");
		goto add_ldarg;
	    default:
		++errcount;
		unsupported(arg, "unknown flag");
		break;
	    }
	}
	else
	{
	    ++nifiles;
	    *argdone = FALSE;
	    length = strlen(arg);
	    if (length >= 2 && arg[length - 2] == '.' &&
		((ext = arg[length - 1]) == 'c' || ext == 's'))
		++ncsfiles;
	}
    }
    nfilters = prep_debug + prep_only + cc_only + as_only;
    if (nfilters != 0)
    {
	if (nfilters > 1)
	{
	    ++errcount;
	    show_who("more than 1 option from -E -P -S -c\n");
	}
	if (nofiles != 0 && ncsfiles > 1)
	{
	    ++errcount;
	    show_who("cannot have more than 1 input with non-linked output\n");
	}
    }
    if (nifiles == 0)
    {
	++errcount;
	show_who("no input files\n");
    }
    if (errcount != 0)
	exit(1);

    path_crtso.target = path_libca.target = bits32_arg[1] = bits32 ? '3' : '0';
    addarg(&ccargs, bits32_arg);
    addarg(&ccargs, path_include.flag);  /* Add after -I... args above, so that it has lower priority. */
    addarg(&asargs, bits32_arg);
    addarg(&ldargs, bits32_arg);
    addarg(&ldargs, path_crtso.libdir);  /* !! Don't add if -nostdlib is specified. */
    addarg(&asargs, "-n");
    if (ncsfiles < 2)
	echo = FALSE;
    set_trap();

    /* pass 2 over argv to compile and assemble .c and .s files */
    /* and gather arguments for loader */
    for (argv -= (argc = argcount) - 1, argdone -= argcount - 1; --argc != 0;)
    {
	arg = *++argv;
	if (!*++argdone)
	{
	    length = strlen(arg);
	    if (length >= 2 && arg[length - 2] == '.' &&
		((ext = arg[length - 1]) == 'c' || ext == 's'))
	    {
		if (echo || verbose)
		{
		    writes(arg);
		    writesn(":");
		}
		if ((basename = strrchr(arg, DIRCHAR)) == NULL)
		    basename = arg;
		else
		    ++basename;
		if (ext == 's')
		    s_out = stralloc(arg);
		else
		{
		    if (cc_only)
		    {
			if (nofiles != 0)
			    s_out = f_out;
			else
			{
			    s_out = stralloc(basename);
			    s_out[strlen(s_out) - 1] = 's';
			}
		    }
		    else
			s_out = my_mktemp();
		    addarg(&ccargs, arg);
		    if (run((strcpy(path_tool.tool, SC), path_tool.libdir), "-o", s_out, &ccargs) != 0)
		    {
			--ccargs.argc;
			status = 1;
			if (!cc_only)
			{
			    --tmpargs.argc;
			    my_unlink(s_out);
			}
			continue;
		    }
		    --ccargs.argc;
		}
		if (!cc_only)
		{
		    if (as_only)
		    {
			if (nofiles != 0)
			    o_out = f_out;
			else
			{
			    o_out = stralloc(basename);
			    o_out[strlen(o_out) - 1] = 'o';
			}
		    }
		    else
			o_out = my_mktemp();
		    arg[length - 1] = 's';
		    addarg(&asargs, arg);
		    addarg(&asargs, s_out);
		    if (run((strcpy(path_tool.tool, AS), path_tool.libdir), "-o", o_out, &asargs) != 0)
			status = 1;
		    asargs.argc -= 2;
		    if (ext == 'c')
		    {
			/* pop o_out over s_out */
			tmpargs.argv[(--tmpargs.argc - 1)] = o_out;
			my_unlink(s_out);
		    }
		    if (!as_only)
			addarg(&ldargs, o_out);
		}
	    }
	    else
		addarg(&ldargs, arg);  /* arg is now typically *.o or *.a */
	}
    }

    if (!cc_only && !as_only && status == 0)
    {
	addarg(&ldargs, path_libca.libdir);  /* !! Don't add if -nostdlib is specified. */
	status = run((strcpy(path_tool.tool, LD), path_tool.libdir), "-o", f_out, &ldargs) != 0;
    }
    killtemps();
    return status;
}

PRIVATE void addarg(argp, arg)
register struct arg_s *argp;
char *arg;
{
    if (argp->size == 0)
	startarg(argp);
    if (++argp->argc >= argp->size &&
	(argp->argv = realloc(argp->argv, (argp->size += ALLOC_UNIT) *
			      sizeof *argp->argv)) == NULL)
	outofmemory("addarg");
    argp->argv[argp->argc - 1] = arg;
    argp->argv[argp->argc] = NULL;
}

PRIVATE void fatal(message)
char *message;
{
    killtemps();
    exit(1);
}

PRIVATE void killtemps()
{
    for (tmpargs.argc -= 2, tmpargs.argv += 2; --tmpargs.argc > 0;)
	my_unlink(*++tmpargs.argv);
}

PRIVATE void *my_malloc(size, where)
unsigned size;
char *where;
{
    void *block;

    if ((block = (void *) malloc(size)) == NULL)
	outofmemory(where);
    return block;
}

PRIVATE char *my_mktemp()
{
    register char *p;
    unsigned digit;
    unsigned digits;
    char *template;
    static unsigned tmpnum;

    p = template = stralloc(TMPNAME);
    p += strlen(p);
    digits = getpid();
    while (*--p == 'X')
    {
	if ((digit = digits % 16) > 9)
	    digit += 'A' - ('9' + 1);
	*p = digit + '0';
	digits /= 16;
    }
    digits = tmpnum;
    while (*p == 'Y')
    {
	if ((digit = digits % 16) > 9)
	    digit += 'A' - ('9' + 1);
	*p-- = digit + '0';
	digits /= 16;
    }
    ++tmpnum;
    addarg(&tmpargs, template);
    return template;
}

PRIVATE void my_unlink(name)
char *name;
{
    if (verbose)
    {
	show_who("unlinking ");
	writesn(name);
    }
    if (unlink(name) < 0 && verbose)
    {
	show_who("error unlinking ");
	writesn(name);
    }
}

PRIVATE void outofmemory(where)
char *where;
{
    show_who("out of memory in ");
    fatal(where);
}

PRIVATE int run(prog, arg1, arg2, argp)
char *prog;
char *arg1;
char *arg2;
register struct arg_s *argp;
{
    int i;
    int status;

    if (argp->size == 0)
	startarg(argp);
    argp->argv[0] = prog;
    argp->argv[1] = arg1;
    argp->argv[2] = arg2;
    if (verbose)
    {
	for (i = 0; i < argp->argc; ++i)
	{
	    writes(argp->argv[i]);
	    writes(" ");
	}
	writen();
    }
    switch (fork())
    {
    case -1:
	show_who("fork failed");
	fatal("");
    case 0:
	execv(prog, argp->argv);
	show_who("exec of ");
	writes(prog);
	fatal(" failed");
    default:
	wait(&status);
	return status;
    }
}

PRIVATE void set_trap()
{
    int signum;

    for (signum = 0; signum <= _NSIG; ++signum)
	if (signal(signum, SIG_IGN) != SIG_IGN)
	    signal(signum, trap);
}

PRIVATE void show_who(message)
char *message;
{
    writes(progname);
    writes(": ");
    writes(message);
}

PRIVATE void startarg(argp)
struct arg_s *argp;
{
    argp->argv = my_malloc((argp->size = ALLOC_UNIT) * sizeof *argp->argv,
			   "startarg");
    argp->argc = 3;
    argp->argv[3] = NULL;
}

PRIVATE char *stralloc(s)
char *s;
{
    return strcpy(my_malloc(strlen(s) + 1, "stralloc"), s);
}

PRIVATE void trap(signum)
int signum;
{
    signal(signum, SIG_IGN);
    if (verbose)
	show_who("caught signal");
    fatal("");
}

PRIVATE void unsupported(option, message)
char *option;
char *message;
{
    show_who("compiler option ");
    writes(option);
    writes(" (");
    writes(message);
    writesn(") not supported yet");
}

PRIVATE void writen()
{
    writes("\n");
}

PRIVATE void writes(s)
char *s;
{
    write(2, s, strlen(s));
}

PRIVATE void writesn(s)
char *s;
{
    writes(s);
    writen();
}
