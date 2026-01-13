/* ld.c - linker for Introl format (6809 C) object files 8086/80386 */

#ifdef STDC_HEADERS_MISSING
extern int errno;
#else
#include <errno.h>
#endif

#include "const.h"
#include "type.h"
#include "globvar.h"

#define NAME_MAX 14

PRIVATE bool_t flag[128];  /* !! Use a smaller array on an ANSI system. */
PRIVATE char libdir[] = "/usr/local/lib/";
PRIVATE char lib86subdir[] = "i86/";
PRIVATE char lib386subdir[] = "i386/";
PRIVATE char lib[sizeof libdir - 1 + sizeof lib386subdir - 1 + NAME_MAX + 1];
PRIVATE char libprefix[] = "lib";
PRIVATE char libsuffix[] = ".a";
long text_base_address;

PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    register char *arg;
    int argn;
    char *outfilename;

    ioinit(argv[0]);
    objinit();
    syminit();
    typeconv_init(0  /* LD_BIG_ENDIAN */, 0  /* LONG_BIG_ENDIAN */);  /* !! size optimization: Hardcode these 0s to typeconv.c. */
    flag['3'] = sizeof(char *) >= 4;
    outfilename = NULL;
    for (argn = 1; argn < argc; ++argn)
    {
	arg = argv[argn];
	if (*arg != '-')
	    readsyms(arg);
	else
	    switch (arg[1])
	    {
	    case '0':		/* use 16-bit libraries */
	    case '3':		/* use 32-bit libraries */
	    case 'M':		/* print symbols linked */
	    case 'i':		/* separate I & D output */
	    case 'm':		/* print modules linked */
	    case 's':		/* strip symbols */
	    case 'z':		/* unmapped zero page */
		if (arg[2] == 0)
		    flag[(unsigned char)arg[1]] = TRUE;
		else if (arg[2] == '-' && arg[3] == 0)
		    flag[(unsigned char)arg[1]] = FALSE;
		else
		    usage();
		if (arg[1] == '0')	/* flag 0 is negative logic flag 3 */
		    flag['3'] = !flag['0'];
		break;
	    case 'T':		/* text base address */
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		errno = 0;    
		text_base_address = strtoul(argv[argn], (char **) NULL, 16);
		if (errno != 0)
		    fatalerror("invalid text address");
		break;
	    case 'l':		/* library name */
		strcpy(lib, libdir);
		strcat(lib, flag['3'] ? lib386subdir : lib86subdir);
		strncat(lib, libprefix, NAME_MAX + 1);
		strncat(lib, arg + 2, NAME_MAX - (sizeof libprefix - 1)
					       - (sizeof libsuffix - 1));
		strcat(lib, libsuffix);
		readsyms(lib);
		break;
	    case 'o':		/* output file name */
		if (arg[2] != 0 || ++argn >= argc || outfilename != NULL)
		    usage();
		outfilename = argv[argn];
		break;
	    default:
		usage();
	    }
    }
    linksyms(flag['r']);
    if (outfilename == NULL)
	outfilename = "a.out";
    writebin(outfilename, flag['i'], flag['3'], flag['s'], flag['z']);
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
