/* strncat.c - char *strncat(char *s1, char *s2, size_t n) */

/* strncat  appends up to  n  characters  from  s2  to the end of  s1 */
/* it returns s1 */

#include <string.h>

char *strncat(s1, s2, n)
char *s1;
_CONST char *s2;
size_t n;
{
    char *initial_s1;

    initial_s1 = s1;
    if (n > 0)
    {
	while (*s1++ != 0)
	    ;
	--s1;
	while ((*s1++ = *s2++) != 0)
	    if (--n == 0)
	    {
		*s1 = 0;
		break;
	    }
    }
    return initial_s1;
}
