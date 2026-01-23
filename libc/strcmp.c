/* strcmp.c - int strcmp(char *s1, char *s2)*/

/* strcmp  compares  s1  to  s2  (lexicographically with unsigned chars) */
/* it returns */
/*		positive  if  s1 > s2 */
/*		zero      if  s1 = s2 */
/*		negative  if  s1 < s2 */

#include <string.h>

int strcmp(s1, s2)
_CONST char *s1;
_CONST char *s2;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    unsigned char ch;

    while ((ch = *s1++) != 0)
	if ((ch -= *s2++) != 0)
	    return ch;
    return ch - *s2;
#else /* !C_CODE etc */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	bx,si
	mov	dx,di
# endif
# if __FIRST_ARG_IN_AX__
	xchg	si,ax
	pop	cx
	pop	di
	dec	sp
	dec	sp
# else
	pop	cx
	pop	si
	pop	di
	sub	sp,#4
# endif
	sub	ax,ax
STRCMP_LOOP:
	lodsb
	test	al,al
	je	STRCMP_EXIT
	scasb
	je	STRCMP_LOOP
	dec	di
STRCMP_EXIT:
	sub	al,[di]
	sbb	ah,#0
# if !__CALLER_SAVES__
	mov	si,bx
	mov	di,dx
# endif
	jmp	cx
# endasm
#endif /* __AS386_16__ */

#if __AS386_32__
# asm
# if !__CALLER_SAVES__
	mov	ecx,esi
	mov	edx,edi
# endif
# if __FIRST_ARG_IN_AX__
	xchg	esi,eax
# else
	mov	esi,_strcmp.s1[esp]
# endif
	mov	edi,_strcmp.s2[esp]
	sub	eax,eax
STRCMP_LOOP:
	lodsb
	test	al,al
	je	STRCMP_EXIT
	scasb
	je	STRCMP_LOOP
	dec	edi
STRCMP_EXIT:
	sub	al,[edi]
	sbb	ah,#0
	cwde
# if !__CALLER_SAVES__
	mov	esi,ecx
	mov	edi,edx
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
