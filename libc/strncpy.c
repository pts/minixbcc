/* strncpy.c - char *strncpy(char *s1, char *s2, size_t n) */

/* strncpy  writes exactly  n  (or 0 if n < 0)  characters to  s1 */ 
/* it copies up to  n  characters from  s2, and null-pads the rest */
/* the result is null terminated iff  strlen( s2 ) < n */
/* it returns the target string */

#include <string.h>

char *strncpy(s1, s2, n)
char *s1;
_CONST char *s2;
size_t n;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    char *initial_s1;

    initial_s1 = s1;
    if (n > 0)
    {
	while ((*s1++ = *s2++) && --n != 0)	
	    ;
	while (n-- != 0)
	    *s1++ = 0;
    }
    return initial_s1;
#else /* !C_CODE etc */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	dx,di
	mov	bx,si
# endif
# if __FIRST_ARG_IN_AX__
	xchg	di,ax
	pop	ax
	pop	si
	pop	cx
	sub	sp,#4
# else
	pop	ax
	pop	di
	pop	si
	pop	cx
	sub	sp,#6
# endif
	push	ax
	push	di		| to return
	jcxz	STNCPY_EXIT
STNCPY_LOOP:
	lodsb
	stosb
	test	al,al
	loopnz	STNCPY_LOOP
	rep
	stosb
STNCPY_EXIT:
	pop	ax
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
	push	eax
	xchg	edi,eax
	mov	esi,TEMPS+_strncpy.s2+4[esp]
	mov	ecx,TEMPS+_strncpy.n+4[esp]
# else
	mov	edi,TEMPS+_strncpy.s1[esp]
	mov	esi,TEMPS+_strncpy.s2[esp]
	mov	ecx,TEMPS+_strncpy.n[esp]
# endif
	jecxz	STNCPY_EXIT
STNCPY_LOOP:
	lodsb
	stosb
	test	al,al
	loopnz	STNCPY_LOOP
	rep
	stosb
STNCPY_EXIT:
# if __FIRST_ARG_IN_AX__
	pop	eax
# else
	mov	eax,TEMPS+_strncpy.s1[esp]
# endif
# if !__CALLER_SAVES__
	pop	esi
	mov	edi,edx
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
