/* memset.c - fill memory with a given char */

#include <string.h>

void *memset(s, ch, n)
void *s;
char ch;
register size_t n;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    register char *cs;

    for (cs = s; n-- != 0;)
	*cs++ = ch;
    return s;
#else /* !C_CODE etc */

#if __AS386_16__
# asm
# if !__CALLER_SAVES__
	mov	dx,di
# endif
# if __FIRST_ARG_IN_AX__
	xchg	di,ax
	pop	bx
	pop	ax
	pop	cx
	sub	sp,#4
# else
	pop	bx
	pop	di
	pop	ax
	pop	cx
	sub	sp,#6
# endif
	push	bx
	mov	bx,di			| to return
	cmp	cx,#8
	jb	MS_SMALL
	mov	ah,al
	test	di,#1
	je	MS_ALIGNED
	stosb
	dec	cx
MS_ALIGNED:
	shr	cx,1
	rep
	stosw
	rcl	cx,1
MS_SMALL:
	rep
	stosb
	xchg	ax,bx
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
	push	eax			| to return
	xchg	edi,eax
	mov	al,TEMPS+_memset.ch+4[esp]
	mov	ecx,TEMPS+_memset.n+4[esp]
# else
	mov	edi,TEMPS+_memset.s[esp]
	mov	al,TEMPS+_memset.ch[esp]
	mov	ecx,TEMPS+_memset.n[esp]
# endif
	cmp	ecx,#17
	jb	MS_SMALL

	mov	ah,al
	shrd	edx,eax,16
	shld	eax,edx,16

	mov	edx,ecx
	mov	ecx,edi		| align
	neg	ecx
	and	ecx,#3		| count for alignment
	sub	edx,ecx
	rep
	stosb
	mov	ecx,edx
	shr	ecx,2		| count of dwords
# if !__CALLER_SAVES__
	push	esi
# endif
	mov	esi,edi		| set up for movs which is 1 cycle faster
	stosd
	dec	ecx
	rep
	movsd			| propagate 1st zero dword
# if !__CALLER_SAVES__
	pop	esi
# endif
	and	edx,#3
	mov	ecx,edx
MS_SMALL:
	rep
	stosb
# if __FIRST_ARG_IN_AX__
	pop	eax
# else
	mov	eax,TEMPS+_memset.s[esp]
# endif
# if !__CALLER_SAVES__
	pop	edi
# endif
# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
