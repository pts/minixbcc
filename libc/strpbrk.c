/* strpbrk.c */

/* from Schumacher's Atari library, improved */

#include <string.h>

char *strpbrk(string, set)
register _CONST char *string;
_CONST char *set;
/*
 *	Return a pointer to the first occurance in <string> of any
 *	character in <set>.
 */
{
    _CONST register char *setptr;

    while (*string)
    {
	setptr = set;
	do
	    if (*setptr == *string)
		return (char *) string;
	while (*setptr++);
	++string;
    }
    return 0;
}
