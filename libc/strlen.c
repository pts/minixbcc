/* strlen.c - size_t strlen(char *s) */

/* strlen  returns the number of characters before the first null in  s */

#include <string.h>

size_t strlen(s)
_CONST char *s;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    _CONST char *start;

    start = s;
    while (*s++ != 0)
	;
    return (s - 1) - start;
#else /* !C_CODE etc */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	dx,di
# endif
# if __FIRST_ARG_IN_AX__
	xchg	di,ax
# else
	mov	bx,sp
	mov	di,_strlen.s[bx]
# endif
	mov	bx,di		| record start
	sub	ax,ax
	mov	cx,#0xFFFF
	repnz
	scasb
	lea	ax,-1[di]
	sub	ax,bx
# if !__CALLER_SAVES__
	mov	di,dx
# endif
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__CALLER_SAVES__
	push	edi
#  define TEMPS 4
# else
#  define TEMPS 0
# endif
# if __FIRST_ARG_IN_AX__
	xchg	edi,eax
# else
	mov	edi,TEMPS+_strlen.s[esp]
# endif
	mov	edx,edi
	sub	eax,eax
	lea	ecx,-1[eax]
	repnz
	scasb
	lea	eax,-1[edi]
	sub	eax,edx
# if !__CALLER_SAVES__
	pop	edi
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
