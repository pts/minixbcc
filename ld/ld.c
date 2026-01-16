/* ld.c - linker for Introl format (6809 C) object files 8086/80386 */

#include <string.h>
#include "const.h"
#include "type.h"
#include "globvar.h"

#define LIBNAME_MAX 32  /* Number of bytes without the directory name, the lib prefix and the .a suffix and the trailing NUL. */

PUBLIC long text_base_address;

PRIVATE bool_t flag[128];  /* !! Use a smaller array on an ANSI system. */
PRIVATE char libdir[] = "/usr/local/lib/";
PRIVATE char lib86subdir[] = "i86/";
PRIVATE char lib386subdir[] = "i386/";
PRIVATE char libtmp[sizeof libdir - 1 + sizeof lib386subdir - 1 + LIBNAME_MAX + 1];
PRIVATE char libprefix[] = "lib";
PRIVATE char libsuffix[] = ".a";

/* This is similar to strtoul(argv[argn], (char **) NULL, 16), but it avoids
 * dealing with errno. There are some differences: it allows only spaces or
 * tabs as whitespace, it fails for trailing junk bytes.
 */
PRIVATE bool_pt parse_u_hex(s, output)
char *s;
long *output;
{
  register char c;
  unsigned count;

  *output = 0;
  for (; (c = s[0]) == ' ' || c == '\t'; ++s) {}
  if (s[0] == '0' && ((c = s[1]) == 'x' || c == 'X')) s += 2;
  if (s[0] == '\0') return 0;  /* Number missing. */
  for (; s[0] == '0'; ++s) {}  /* Don't count leading '0's as digits for overflow. */
  count = 0;
  while ((c = *s++) != '\0') {
    if (++count == (sizeof(long) << 1) + 1) return 0;  /* Overflow in number of digits. */
    if (c >= '0' && c <= '9') {
      c -= '0';
    } else if (c >= 'a' && c <= 'f') {
      c -= 'a' - 10;
    } else if (c >= 'A' && c <= 'F') {
      c -= 'A' - 10;
    } else {
      return 0;  /* Bad hex digit in c. */
    }
    *output <<= 4;
    *output += c;
  }
  return *output >= 0;
}

PRIVATE char *appendstr(dest, src)
char *dest;
char *src;
{
	return strcpy(dest, src) + strlen(src);
}

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
		if (!parse_u_hex(argv[argn], &text_base_address))
		    fatalerror("invalid text address");
		break;
	    case 'l':		/* library name */
		if (strlen(arg) > LIBNAME_MAX + 1) {
		    refer();
		    putstr("library name too long ");
		    errexit(arg);
		}
		appendstr(appendstr(appendstr(appendstr(appendstr(libtmp,
		    libdir),
		    flag['3'] ? lib386subdir : lib86subdir),
		    libprefix),
		    arg + 2),
		    libsuffix);
		readsyms(libtmp);
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
