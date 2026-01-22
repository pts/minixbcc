/* ld.c - linker for Introl format (6809 C) object files 8086/80386 */

#ifdef LIBCH
#  include "libc.h"
#else
#  include <string.h>
#endif
#include "const.h"
#include "type.h"
#include "globvar.h"

PRIVATE bool_t flag[128];  /* !! Use a smaller array on an ANSI system. */

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
    outfilename = (char*) 0;
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
	    case 'T':		/* text base address; default: 0 */
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		/* Earlier versions of ld (such as early v1) had base == 16 below. */
		if (!parse_nonneg_lenient(argv[argn], 0  /* base */, &btextoffset))
		    fatalerror("invalid text address");
		break;
	    case 'h':		/* dynamic memory size (including heap, stack, argv and environ); the default of 0 means automatic */
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		if (!parse_nonneg_lenient(argv[argn], 0  /* base */, &dynam_size))
		    fatalerror("invalid dynamic memory size");
		break;
	    case 'o':		/* output file name */
		if (arg[2] != 0 || ++argn >= argc || outfilename != (char*) 0)
		    usage();
		outfilename = argv[argn];
		break;
	    default:
		usage();
	    }
    }
    linksyms(flag['r']);
    if (outfilename == (char*) 0)
	outfilename = "a.out";
    writebin(outfilename, flag['i'], flag['3'], flag['s'], flag['z']);
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
