/* cc.c - driver for minixbcc (v3) BCC (Bruce's C compiler) */

#define _POSIX_SOURCE  /* For the Minix libc. */
#define _BSD_SOURCE  /* For glibc <=2.19 and minilibc686 reliable signal(...). */
#define _DEFAULT_SOURCE  /* For glibc >=2.19 and minilibc686 reliable signal(...). */
#define CONFIG_SIGNAL_BSD  /* For olde minilibc686 reliable signal(...) (bsd_signal(...)). */


#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>

#ifdef __WATCOMC__  /* OpenWatcom v2 or earlier. */
#  ifdef _WCDATA  /* OpenWatcom v2 libc. */
#    ifdef __LINUX__  /* owcc -blinux */
#      define CUSTOMSYSWAITH  /* Workaround for `sh build.sh owcc', without the `-I"$WATCOM"/lh' or `export INCLUDE=$WATCOM/lh', and using (by default) -I"$WATCOM"/h instead of the correct -I"$WATCOM"/lh on Linux. */
#      define WATCOMLINUXSYSWAITH
#    endif
#  endif
#endif
#ifndef CUSTOMSYSWAITH
#  include <sys/wait.h>
#endif

#ifdef WATCOMLINUXSYSWAITH
#  define WIFEXITED(s) (((s) & 0377) == 0)
#  define WEXITSTATUS(s) (((s) >> 8) & 0377)
  extern pid_t wait(int *_stat_loc);
  extern pid_t fork(void);
#endif

#ifdef NOCROSS
#  define CROSS 0
#  define GET_PATH_TOOL(tool_name) (strcpy(path_tool.tool, tool_name), path_tool.libdir)
#  define GET_PATH_INCLUDE() flag_include.flag
#  define GET_PATH_TARGET_CRTSO() path_crtso.libdir
#  define GET_PATH_TARGET_LIBCA() path_libca.libdir
#  define IS_HOST_BITS32 (sizeof(char *) >= 4)
#else
#  define CROSS 1
#  define GET_PATH_TOOL(tool_name) get_cross_path_tool((tool_name))
#  define GET_PATH_INCLUDE() get_cross_flag_include()
#  define GET_PATH_TARGET_CRTSO() get_cross_path_target(CRTSO, &target_CRTSO)
#  define GET_PATH_TARGET_LIBCA() get_cross_path_target(LIBCA, &target_LIBCA)
#  define IS_HOST_BITS32 1
#endif

#define FALSE	0
#define TRUE	1

#ifndef   FORWARD
#  define FORWARD static
#endif
#ifndef   PRIVATE
#  define PRIVATE static
#endif
#define PUBLIC

#if __STDC__
#  define _CONST const
#  define P(x) x
#  define P0() (void)
#  define P1(t1, n1) (t1 n1)
#  define P2(t1, n1, t2, n2) (t1 n1, t2 n2)
#  define P3(t1, n1, t2, n2, t3, n3) (t1 n1, t2 n2, t3 n3)
#  define P4(t1, n1, t2, n2, t3, n3, t4, n4) (t1 n1, t2 n2, t3 n3, t4 n4)
#else
#  define _CONST
#  define P(x) ()
#  define P0() ()
#  define P1(t1, n1) (n1) t1 n1;
#  define P2(t1, n1, t2, n2) (n1, n2) t1 n1; t2 n2;
#  define P3(t1, n1, t2, n2, t3, n3) (n1, n2, n3) t1 n1; t2 n2; t3 n3;
#  define P4(t1, n1, t2, n2, t3, n3, t4, n4) (n1, n2, n3, n4) t1 n1; t2 n2; t3 n3; t4 n4;
#endif
#if __cplusplus >= 201703L
#  define REGISTER   /* register removed in C++17. */
#else
#  define REGISTER register
#endif

typedef unsigned char bool_t;	/* boolean: TRUE if nonzero */

#define LIBDIR "/usr/minixbcc"
#define INCLUDE "include"

#define SC	"sc"
#define AS	"as"
#define LD	"ld"

#define CRTSO	"crtso.o"
#define LIBCA	"libc.a"

#if CROSS
#  define TMPDIRDEFAULT "/tmp"
#  define TMPBASEPATH "/bbccXXXXXXXX"
#else
#  define TMPNAME "/tmp/bbccXXXXXXXX"
#endif

#define ALLOC_UNIT	16	/* allocation unit for arg arrays */
#define DIRCHAR	'/'

#define TOOL_SIZE 15  /* Maximum number of bytes in the basename of the tool, including the trailing NUL. */

struct arg_s
{
    int argc;
    _CONST char **argv;
    unsigned size;
};

PRIVATE struct arg_s asargs;	/* = NULL */
PRIVATE struct arg_s scargs;	/* = NULL */
PRIVATE struct arg_s ldargs;	/* = NULL */
PRIVATE char *progname;
PRIVATE struct arg_s tmpargs;	/* = NULL */
PRIVATE bool_t verbose;		/* = FALSE */
PRIVATE char bits32_arg[3] = "-?";  /*The '?' will be replaced with '0' or '3'. */

FORWARD void addarg P((REGISTER struct arg_s *argp, _CONST char *arg));
FORWARD void fatal P((void));
FORWARD void killtemps P((void));
FORWARD void *my_malloc P((unsigned size, _CONST char *where));
FORWARD char *my_mktemp P((void));
FORWARD void my_unlink P((_CONST char *name));
FORWARD void outofmemory P((_CONST char *where));
FORWARD int execute P((_CONST char **argv));
FORWARD int run P((_CONST char *prog, _CONST char *arg1, _CONST char *arg2, struct arg_s *argp));
FORWARD void set_trap P((void));
FORWARD void show_who P((_CONST char *message));
FORWARD void startarg P((struct arg_s *argp));
FORWARD char *stralloc P((_CONST char *s));
FORWARD void trap P((int signum));
FORWARD void unsupported P((_CONST char *option, _CONST char *message));
FORWARD void writen P((void));
FORWARD void writes P((_CONST char *s));
FORWARD void writesn P((_CONST char *s));
FORWARD bool_t is_tool P((_CONST char *s));
FORWARD void trap_signal P((int signum));
FORWARD unsigned long mix3 P((unsigned long key));

#if CROSS
PRIVATE char *driverdir;  /* Directory of this driver program (from its argv[0]). */
PRIVATE unsigned driverdirlen;  /* Number of valid bytes starting at driverdir. */
PRIVATE char *path_tool_;
PRIVATE char *flag_include_;
PRIVATE char *target_CRTSO;
PRIVATE char *target_LIBCA;
PRIVATE void init_driverdir P((void));
FORWARD char *get_cross_path_tool P((_CONST char *tool_name));
FORWARD char *get_cross_flag_include P((void));
FORWARD char *get_cross_path_target P((_CONST char *file_name, char **file_cache));
#else
/* We need this workaround since many old C compilers, including BCC con't support string literal concatenation in:
 * char flag_include[] = "-I" LIBDIR INCLUDE;
 */
PRIVATE struct
{
    char flag[2];
    char libdir[sizeof(LIBDIR)];  /* The trailing '\0' will be replaced with '/'. */
    char include[sizeof(INCLUDE)];
} flag_include = { {'-', 'I'}, LIBDIR, INCLUDE };

PRIVATE struct
{
    char libdir[sizeof(LIBDIR)];  /* The trailing '\0' will be replaced with '/'. */
    char host;  /* '0' or '3' */
    char slash;  /* '/' */
    char tool[TOOL_SIZE];  /* 4 would be enough for cpp, but being generous with 15 for custom tools. */
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
#endif

PUBLIC int main P((int argc, char **argv));

PUBLIC int main P2(int, argc, char **, argv)
{
    REGISTER char *arg;
    int argcount = argc;
    bool_t *argdone;
    bool_t as_only = FALSE;
    char *basename;
    bool_t bits32 = IS_HOST_BITS32;
    bool_t sc_only = FALSE;
    bool_t echo = FALSE;
    unsigned errcount = 0;
    char ext;
    _CONST char *f_out = "a.out";
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

    progname = argv[0];
#if CROSS
    init_driverdir();
#else
    path_tool.libdir[sizeof(path_tool.libdir) - 1] =
        flag_include.libdir[sizeof(flag_include.libdir) - 1] =
        path_crtso.libdir[sizeof(path_crtso.libdir) - 1] =
        path_libca.libdir[sizeof(path_libca.libdir) - 1] = '/';
#endif

    if (is_tool(arg = argv[1]))  /* Example: bbcc ld -3 -o foo.o foo.s */
    { runtool:
#if CROSS
	*++argv = get_cross_path_tool(arg);
#else
	if (strlen(arg) > sizeof(path_tool.tool) - 1)
	{
		show_who("tool name too long: ");
		writesn(arg);
		fatal();
	}
	strcpy(path_tool.tool, arg);
	*++argv = path_tool.libdir;
#endif
	status = execute((_CONST char **) argv);
	return WIFEXITED(status) ? WEXITSTATUS(status) : 126;
    }
    if (arg[0] == '-' && arg[1] == 'v' && arg[2] == 0 && is_tool(arg = argv[2]))  /* Example: bbcc -v ld -3 -o foo.o foo.s */
    {
	++argv;
	verbose = TRUE;
	goto runtool;
    }

    argdone = (bool_t *) my_malloc((unsigned) argc * sizeof *argdone, "argdone");
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
		/* unsupported( arg, "optimize" ); */  /* Ignore, BCC doesn't support optimization. */
		break;
	    case 'S':
		sc_only = TRUE;
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
		/* debug = TRUE; */	/* unsupported( arg, "debug" ); */
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
		if (argc == 1)
		{ missing_parameter:
		    ++errcount;
		    show_who("missing value for ");
		    writesn(arg);
		    break;
		}
		addarg(&scargs, arg);
		--argc;
		addarg(&scargs, *++argv);  /* sc supports receiving these flags (and also -o) as 1 or 2 arguments. Here we can only send as 2. */
		*++argdone = TRUE;
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
		unsupported(arg, "substituted sc");
		break;
	    case 'C':
		addarg(&scargs, argval);
		break;
	    case 'D':
	    case 'U':
	    case 'I':
		addarg(&scargs, arg);
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
    nfilters = prep_debug + prep_only + sc_only + as_only;
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

#if CROSS==0
    path_crtso.target = path_libca.target =
#endif
        bits32_arg[1] = bits32 ? '3' : '0';
    addarg(&scargs, bits32_arg);
    addarg(&scargs, GET_PATH_INCLUDE());  /* Add after -I... args above, so that it has lower priority. */
    addarg(&asargs, bits32_arg);
    addarg(&ldargs, bits32_arg);
    addarg(&ldargs, GET_PATH_TARGET_CRTSO());  /* !! Don't add if -nostdlib is specified. */
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
		if ((basename = strrchr(arg, DIRCHAR)) == (char*) 0)
		    basename = arg;
		else
		    ++basename;
		if (ext == 's')
		{
		    if (!sc_only) s_out = stralloc(arg);
		}
		else
		{
		    if (sc_only)
		    {
			if (nofiles != 0)
			    s_out = stralloc(f_out);
			else
			{
			    s_out = stralloc(basename);
			    s_out[strlen(s_out) - 1] = 's';
			}
		    }
		    else
			s_out = my_mktemp();
		    addarg(&scargs, arg);
		    if (run(GET_PATH_TOOL(SC), "-o", s_out, &scargs) != 0)
		    {
			--scargs.argc;
			status = 1;
			if (!sc_only)
			{
			    --tmpargs.argc;
			    my_unlink(s_out);
			}
			continue;
		    }
		    --scargs.argc;
		}
		if (!sc_only)
		{
		    if (as_only)
		    {
			if (nofiles != 0)
			    o_out = stralloc(f_out);
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
		    if (run(GET_PATH_TOOL(AS), "-o", o_out, &asargs) != 0)
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

    if (!sc_only && !as_only && status == 0)
    {
	addarg(&ldargs, GET_PATH_TARGET_LIBCA());  /* !! Don't add if -nostdlib is specified. */
	status = run(GET_PATH_TOOL(LD), "-o", f_out, &ldargs) != 0;
    }
    killtemps();
    return status;
}

#if CROSS
#define LIBEXECPATH "/../libexec/"
#define TARGETPATH "/../?/"
#define INCLUDEDIR  "/../include"
PRIVATE void init_driverdir P0()
{
    REGISTER char *p, *q;

    for (driverdir = p = progname, q = p + strlen(p); q != p && q[-1] != '/'; --q) {}
    if (q == p) {
	show_who("missing directory in argv[0]");  /* !! Look up argv[0] on $PATH, get the directory from there. */
	writen();
	fatal();
    }
    for (; q != p && q[-1] == '/'; --q) {}
    driverdirlen = q - p;
}
PRIVATE char *get_cross_path_tool P1(_CONST char *, tool_name)
{
    REGISTER char *p;

    if (strlen(tool_name) > (unsigned) (TOOL_SIZE - 1)) {
	show_who("tool name too long: ");
	writesn(tool_name);
	fatal();
    }
    if (!(p = path_tool_)) {
	path_tool_ = p = (char*) my_malloc(driverdirlen + sizeof(LIBEXECPATH) + TOOL_SIZE - 1, "tool");
	memcpy(p, driverdir, driverdirlen);
	memcpy(p += driverdirlen, LIBEXECPATH, sizeof(LIBEXECPATH) - 1);
    } else {  /* Reuse the same path_tool_ buffer. */
	p += driverdirlen;
    }
    strcpy(p + sizeof(LIBEXECPATH) - 1, tool_name);
    return path_tool_;
}
PRIVATE char *get_cross_flag_include P0()
{
    REGISTER char *p;

    if (!(p = flag_include_)) {
	flag_include_ = p = (char*) my_malloc(2 + driverdirlen + sizeof(INCLUDEDIR), "include flag");
	*p++ = '-'; *p++ = 'I';
	memcpy(p, driverdir, driverdirlen);
	memcpy(p + driverdirlen, INCLUDEDIR, sizeof(INCLUDEDIR));
	p = flag_include_;
    }
    return p;
}
PRIVATE char *get_cross_path_target P2(_CONST char *, file_name, char **, file_cache)
{
    REGISTER char *p;

    if (!(p = *file_cache)) {
	*file_cache = p = (char*) my_malloc(driverdirlen + sizeof(TARGETPATH) + strlen(file_name), "target-specific file");
	memcpy(p, driverdir, driverdirlen);
	memcpy(p += driverdirlen, TARGETPATH, sizeof(TARGETPATH) - 1);
	p += sizeof(TARGETPATH) - 1; p[-2] = bits32_arg[1];  /* Replace the '?' in the copy of TARGETPATH with target-specific '0' or '3'. */
	strcpy(p, file_name);
	p = *file_cache;
    }
    return p;
}
#endif

PRIVATE void addarg P2(REGISTER struct arg_s *, argp, _CONST char *, arg)
{
    if (argp->size == 0)
	startarg(argp);
    if (++argp->argc >= (int) argp->size &&
	(argp->argv = (_CONST char **) realloc(argp->argv, (argp->size += ALLOC_UNIT) * sizeof *argp->argv)) == (_CONST char **) 0)
	outofmemory("addarg");
    argp->argv[argp->argc - 1] = arg;
    argp->argv[argp->argc] = (char*) 0;
}

PRIVATE void fatal P0()
{
    killtemps();
    exit(1);
}

PRIVATE void killtemps P0()
{
    for (tmpargs.argc -= 2, tmpargs.argv += 2; --tmpargs.argc > 0;)
	my_unlink(*++tmpargs.argv);
}

PRIVATE void *my_malloc P2(unsigned, size, _CONST char *, where)
{
    void *block;

    if ((block = (void *) malloc(size)) == (void*) 0)  /* !! Implement a much simpler, no-free() allocator for Minix host. */  /* !! Use smaller malloc() for minilibc686. */
	outofmemory(where);
    return block;
}

/*
 * mix3 is a period 2**32-1 PNRG ([13,17,5]).
 *
 * https://stackoverflow.com/a/54708697
 * https://stackoverflow.com/a/70960914
 *
 * This xorshift technique is based on the 2003 paper titled ``Xorshift
 * PRNGs'' by George Marsaglia, published in the Journal of Statistical
 * Software. It ensures that entropy from high-order bits propagates to
 * low-order bits and vice versa.
 *
 * The iteration count of 10 was chosen empirically by looking at key
 * values 0..19 and the upper 2 and 3 bits of mixes3(key). Even 6 and 7 are
 * bad, 9 is much better, 10 is good enough.
 *
 * This function uses the low 32 bits of the input, and returns a value less
 * than 1 << 32.
 */
PRIVATE unsigned long mix3 P1(unsigned long, key)
{
    key ^= (key << 13);
    key ^= ((key & (unsigned long) 0xffffffffL) >> 17);
    key ^= (key << 5);
    return key & (unsigned long) 0xffffffffL;
}

PRIVATE char *my_mktemp P0()
{
    REGISTER char *p;
    unsigned digit;
    unsigned long digits;
    char *tmpfilename;
    static unsigned long tmpnum;
    static char is_tmpnum_valid;
#if CROSS
    static _CONST char *tmpdir;
#endif
    time_t ts;
    int fd;
    int tries_remaining;

#if CROSS
    if (!tmpdir && (!(tmpdir = getenv("TMPDIR")) || tmpdir[0] == '\0')) tmpdir = TMPDIRDEFAULT;
    strcpy(tmpfilename = p = (char*) my_malloc(strlen(tmpdir) + sizeof(TMPBASEPATH), "temporary file"), tmpdir);
    memcpy(p + strlen(p), TMPBASEPATH, sizeof(TMPBASEPATH));
#else
    tmpfilename = stralloc(TMPNAME);
#endif
    if (!is_tmpnum_valid) {
        time(&ts);
        /* !! Use even better techniques to add entropy in https://github.com/pts/minilibc686/blob/master/tools/mktmpf.c */
        tmpnum = mix3(mix3(getpid())) + mix3(ts);
        ++is_tmpnum_valid;
    }
    tries_remaining = 1024;
  again:
    digits = tmpnum = mix3(tmpnum);  /* Generate next temporary filename to try. */
    for (p = tmpfilename, p += strlen(p); *--p == 'X'; digits >>= 4)
    {
	*p = ((digit = (unsigned) digits & 15) > 9) ? digit + 'A' - 10 : digit + '0';
    }
    p = tmpfilename;
    /* Like creat(p), but with `| O_EXCL' added for safety, i.e. to avoid the data race with another process. */
    if ((fd = open(p, O_CREAT | O_WRONLY | O_TRUNC | O_EXCL, 0666)) < 0) {
	if (tries_remaining-- == 0) {
	    show_who("error creating temporary files, stopped at ");
	    writesn(p);
	    fatal();
	}
	goto again;
    }
    close(fd);
    addarg(&tmpargs, p);
    return p;
}

PRIVATE void my_unlink P1(_CONST char *, name)
{
    struct stat st;

    if (verbose)
    {
	show_who("unlinking ");
	writesn(name);
    }
    if (unlink(name) < 0 && verbose)
    {
	if (stat(name, &st) == 0) {
	    show_who("error unlinking ");
	    writesn(name);
	}
    }
}

PRIVATE void outofmemory P1(_CONST char *, where)
{
    show_who("out of memory in ");
    writesn(where);
    fatal();
}

#ifdef __MINILIBC686__  /* For old minilibc686. The new one has execv(...) */
extern char **environ;
#endif

PRIVATE int execute P1(_CONST char **, argv)
{
    int status;
    REGISTER _CONST char **argvi;

    if (verbose) {
	for (argvi = argv; *argvi; ++argvi)
	{
	    writes(*argvi);
	    writes(" ");
	}
	writen();
    }
    switch (fork())
    {
    case -1:
	show_who("fork failed\n");
	fatal();
	/* Fallthrough. */  /* Not reached. */
    case 0:  /* Child. */
#ifdef __MINILIBC686__  /* For old minilibc686. The new one has execv(...) */
	execve(argv[0], argv, environ);
#else
#  ifdef __cplusplus
	execv(argv[0], (char * _CONST *) argv);
#  else
	execv((void *) argv[0], (void *) argv);  /* (void *) to cast away constness. C++ is picky, we can't do it there. */
#  endif
#endif
	tmpargs.argc = 0;  /* Make killtemps() a no-op. The parent will delete the temporary files. */
	show_who("exec of ");
	writes(argv[0]);
	writes(" failed\n");
	fatal();
	/* Fallthrough. */  /* Not reached. */
    default:
	wait(&status);
	return status;
    }
}

PRIVATE int run P4(_CONST char *, prog, _CONST char *, arg1, _CONST char *, arg2, REGISTER struct arg_s *,argp)
{
    if (argp->size == 0)
	startarg(argp);
    argp->argv[0] = prog;
    argp->argv[1] = arg1;
    argp->argv[2] = arg2;
    return execute(argp->argv);
}

PRIVATE void trap_signal P1(int, signum)
{
    if (signal(signum, SIG_IGN) != SIG_IGN)  /* !! use sigaction instead if available, i.e. #ifdef SA_RESTART (which Minix doesn't have) */
	signal(signum, trap);
}

PRIVATE void set_trap P0()
{
#ifdef SIGHUP
    trap_signal(SIGHUP);
#endif
#ifdef SIGINT
    trap_signal(SIGINT);
#endif
#ifdef SIGQUIT
    trap_signal(SIGQUIT);
#endif
#ifdef SIGQUIT
    trap_signal(SIGTERM);
#endif
}

PRIVATE void show_who P1(_CONST char *, message)
{
    writes(progname);
    writes(": ");
    writes(message);
}

PRIVATE void startarg P1(struct arg_s *, argp)
{
    argp->argv = (_CONST char **) my_malloc((argp->size = ALLOC_UNIT) * sizeof *argp->argv, "startarg");
    argp->argc = 3;
    argp->argv[3] = (char*) 0;
}

PRIVATE char *stralloc P1(_CONST char *, s)
{
    return strcpy((char *) my_malloc(strlen(s) + 1, "stralloc"), s);
}

PRIVATE void trap P1(int, signum)
{
    signal(signum, SIG_IGN);
    if (verbose)
	show_who("caught signal\n");
    fatal();
}

PRIVATE void unsupported P2(_CONST char *, option, _CONST char *, message)
{
    show_who("compiler option ");
    writes(option);
    writes(" (");
    writes(message);
    writesn(") not supported yet");
}

PRIVATE void writen P0()
{
    writes("\n");
}

PRIVATE void writes P1(_CONST char *, s)
{
    (void)!write(2, s, strlen(s));
}

PRIVATE void writesn P1(_CONST char *, s)
{
    writes(s);
    writen();
}

PRIVATE bool_t is_tool P1(REGISTER _CONST char *, s)
{
    if (!s || *s == '-' || *s == '\0') return FALSE;
    for (; *s != '\0'; ++s) {
	if (*s == '/' || *s == '.') return FALSE;
    }
    return TRUE;
}
