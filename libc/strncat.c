/* strncat.c - char *strncat(char *s1, char *s2, size_t n) */

/* strncat  appends up to  n  characters  from  s2  to the end of  s1 */
/* it returns s1 */

#include <string.h>

char *strncat(s1, s2, n)
char *s1;
_CONST char *s2;
size_t n;
{
#if C_CODE || __AS09__ != 1
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
#else /* !C_CODE etc */

#if __AS09__
# asm
	LDU	_strncat.s2,S	s1 already in X
	STX	_strncat.s2,S	remember s1 to return
	LDY	_strncat.n,S
	BEQ	STRNCAT.EXIT
STRNCAT.LOOP.1
	LDB	,X+
	BNE	STRNCAT.LOOP.1
	LEAX	-1,X
STRNCAT.LOOP.2
	LDB	,U+
	STB	,X+
	BEQ	STRNCAT.EXIT
	LEAY	-1,Y
	BNE	STRNCAT.LOOP.2
	CLR	,X	
STRNCAT.EXIT
	LDX	_strncat.s2,S
# endasm
#endif /* __AS09__ */
#endif /* C_CODE etc */
}
