/* strtol.c						ANSI 4.10.1.5
 *	long int strtol(const char *nptr, char **endptr, int base);
 *
 *	Converts a numeric string, in various bases, to a long integer.
 */

#include <lib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#ifdef strtol
#undef strtol
#endif

PUBLIC long int strtol(s, endptr, base)
_CONST char *s;
char **endptr;
int base;
{
  register int c;
  register _CONST char *nptr = s;
  long int result = 0L;
  long int limit;
  int negative = 0;
  int saw_a_digit = 0;			/* it's not a number without a digit */

  while ((c = (unsigned char) *nptr) != 0 && isspace(c))
	++nptr;				/* skip leading white space */

  if (c == '+' || c == '-') {		/* handle signs */
	negative = (c == '-');
	++nptr;
  }

  if (base == 0) {			/* determine base if unknown */
	base = 10;
	if (*nptr == '0') {
		++nptr;
		if ((c = *nptr) == 'x' || c == 'X') {
			++nptr;
			base = 16;
		}
		else {
			saw_a_digit = 1;	/* in case '0' is only digit */
			base = 8;
		}
	}
  }
  else
  if (base == 16 && *nptr == '0') {	/* discard 0x/0X prefix if hex */
	++nptr;
	if ((c = *nptr == 'x') || c == 'X')
		++nptr;
  }

  if (negative) {
	limit = LONG_MIN / base;
	
	/* Insist on the version of division that truncates towards 0. */
#if PARANOIA
	limit += ((limit + 1) * base < LONG_MIN + base);
#else
	limit += (-1 / 2 == -1);
#endif
  }
  else
	limit = LONG_MAX / base;

  --nptr;				/* convert the number */
  while ((c = (unsigned char) *++nptr) != 0) {
	if (isdigit(c))
		c -= '0';
	else
		c -= isupper(c) ? ('A' - 10) : ('a' - 10);
	if (c < 0 || c >= base)
		break;
	saw_a_digit = 1;
	
	/* The negative case is best handled at the lowest level to avoid
	 * overflows and to avoid using implementation-defined casts.
	 */
	if (negative) {
		if (result < limit || (result *= base) < LONG_MIN + c) {
			result = LONG_MIN;
			errno = ERANGE;
		}
		else
			result -= c;
	}
	else {
		if (result > limit || (result *= base) > LONG_MAX - c) {
			result = LONG_MAX;
			errno = ERANGE;
		}
		else
			result += c;
	}
  }

  if (endptr != (char **) NULL)		/* record final pointer */
	*endptr = (char *) (saw_a_digit ? nptr : s);
  return result;
}
