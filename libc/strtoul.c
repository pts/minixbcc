/* strtoul.c						ANSI 4.10.1.6
 *	unsigned long int strtoul(const char *nptr, char **endptr, int base);
 *
 *	Converts a numeric string, in various bases, to an unsigned long.
 */

#include <lib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

#ifdef strtoul
#undef strtoul
#endif

PUBLIC unsigned long int strtoul(s, endptr, base)
_CONST char *s;
char **endptr;
int base;
{
  register int c;
  register _CONST char *nptr = s;
  unsigned long int result = 0L;
  unsigned long int limit;
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

  limit = ULONG_MAX / base;		/* ensure no overflow */

  --nptr;				/* convert the number */
  while ((c = (unsigned char) *++nptr) != 0) {
	if (isdigit(c))
		c -= '0';
	else
		c -= isupper(c) ? ('A' - 10) : ('a' - 10);
	if (c < 0 || c >= base)
		break;
	saw_a_digit = 1;
	if (result > limit || (result *= base) > ULONG_MAX - c) {
		result = ULONG_MAX;
		errno = ERANGE;
		negative = 0;		/* keep ULONG_MAX as the result */
	}
	else
		result += c;
  }
  if (negative)			/* BIZARRE, but ANSI says we should do this! */
	result = 0L - result;

  if (endptr != (char **) NULL)		/* record final pointer */
	*endptr = (char *) (saw_a_digit ? nptr : s);
  return result;
}
