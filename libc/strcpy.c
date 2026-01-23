/* strcpy.c - char *strcpy(char *s1, char *s2) */

/* strcpy  copies  s2  to  s1, up to and including the null at the end of s1 */
/* the strings should not overlap (downwards copies work OK) */
/* it returns the target string */

#include <string.h>

char *strcpy(s1, s2)
char *s1;
_CONST char *s2;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    char *initial_s1;

    initial_s1 = s1;
    while (*s1++ = *s2++)
	;
    return initial_s1;
#else /* !C_CODE etc */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	dx,di
	mov	bx,si
# endif
# if __FIRST_ARG_IN_AX__
	pop	cx
	pop	si
	dec	sp
	dec	sp
# else
	pop	cx
	pop	ax
	pop	si
	sub	sp,#4
# endif
	push	cx
	push	ax
	mov	di,si
	sub	ax,ax
	mov	cx,#0xFFFF
	repnz
	scasb
	inc	cx
	neg	cx
	pop	ax
	mov	di,ax
	rep
	movsb
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
	mov	esi,TEMPS+_strcpy.s2[esp]
	mov	edi,esi
# if __FIRST_ARG_IN_AX__
	push	eax
# endif
	sub	eax,eax		| method with finding length of s2 takes
	lea	ecx,-1[eax]	| 18+12n vs 19n
	repnz
	scasb
	inc	ecx
	neg	ecx
# if __FIRST_ARG_IN_AX__
	pop	eax
# else
	mov	eax,TEMPS+_strcpy.s1[esp]
# endif
	mov	edi,eax
	rep			| it is faster to avoid fancy alignment tests
	movsb			| (5+4n vs 28+[0-12]+[1-3]n+[0-12])
# if !__CALLER_SAVES__
	pop	esi		| but could join memcpy
	mov	edi,edx
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
