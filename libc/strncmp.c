/* strncmp.c - int strncmp(char *s1, char *s2, size_t n)*/

/* strcmp  compares  s1  to  s2, up to at most n characters */
/*         (lexicographically with native char comparison) */
/* it returns */
/*		positive  if  s1 > s2 */
/*		zero      if  s1 = s2 */
/*		negative  if  s1 < s2 */

#include <string.h>

int strncmp(s1, s2, n)
_CONST char *s1;
_CONST char *s2;
size_t n;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    if (n <= 0)
	return 0;
    while (*s1++ == *s2++)
	if (s1[-1] == 0 || --n == 0)
	    return 0;
    return s1[-1] - s2[-1];
#else /* !C_CODE etc */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	bx,si
	mov	dx,di
# endif
# if __FIRST_ARG_IN_AX__
	xchg	si,ax
	pop	ax
	pop	di
	pop	cx
	sub	sp,#4
# else
	pop	ax
	pop	si
	pop	di
	pop	cx
	sub	sp,#6
# endif
	push	ax
	sub	ax,ax		| prepare for various exits
	jcxz	SNCMP_1EXIT
SNCMP_LOOP:
	lodsb
	or	al,al
	je	SNCMP_EXIT
	scasb
	loopz	SNCMP_LOOP
	dec	di
SNCMP_EXIT:
	sub	al,[di]		| ah is 0
	sbb	ah,#0
SNCMP_1EXIT:
# if !__CALLER_SAVES__
	mov	si,bx
	mov	di,dx
# endif
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__CALLER_SAVES__
	mov	edx,edi
	push	esi
#  define TEMPS 4
# else
#  define TEMPS 0
# endif
# if __FIRST_ARG_IN_AX__
	xchg	esi,eax
# else
	mov	esi,TEMPS+_strncmp.s1[esp]
# endif
	mov	edi,TEMPS+_strncmp.s2[esp]
	mov	ecx,TEMPS+_strncmp.n[esp]
	sub	eax,eax		| prepare for various exits
	jecxz	SNCMP_1EXIT
SNCMP_LOOP:
	lodsb
	or	al,al
	je	SNCMP_EXIT
	scasb
	loopz	SNCMP_LOOP
	dec	edi
SNCMP_EXIT:
	sub	al,[edi]
	sbb	ah,#0
	cwde
SNCMP_1EXIT:
# if !__CALLER_SAVES__
	pop	esi
	mov	edi,edx
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
