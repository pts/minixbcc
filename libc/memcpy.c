/* memcpy.c - void *memcpy(void *s1, void *s2, size_t nbytes) */

/* memcpy  copies  nbytes  bytes  from  s2  to  s1 */
/* nothing is copied if  s1 == s2 */
/* overlapping moves are handled correctly (not required by ANSI) */
/* memcpy  returns its target */

#include <string.h>

void *memcpy(s1, s2, nbytes)
void *s1;
_CONST void *s2;
size_t nbytes;
{
#if C_CODE || __AS386_16__ + __AS386_32__ != 1
    register char *cs1;
    register char *cs2;
    unsigned largecount;
    unsigned char smallcount;
    void *s1top;

    -- upwards move only shown in C
    -- only the algorithm for the 6809 assembler version is shown
    -- sorry, this does not compile
    -- although it is easy to write a slow version

    if ((smallcount = nbytes & (8 - 1)) != 0)
    do
	*cs1++ = *cs2++;
    while (--smallcount != 0);
    if ((largecount = nbytes & ~(8 - 1)) != 0)
    {
	s1top = cs1 + largecount;
	do
	{
	    -- copy 8 bytes from s1 to s2 (might use double assignment)
	    cs1 += 8;
	    cs2 += 8;
	}
	while (cs1 < s1top)
    }
    return s1;
#else /* !C_CODE etc */

/*
    80*86 calling conventions (small model only):
	arguments on stack except 1st arg may be in ax
	direction flag clear
	es == ds
	routine must preserve ds, es and direction
	may have to preserve si, di
	return result in ax
*/

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
	push	ax		| must push something, return adr is best!
	mov	ax,di		| to return
	cmp	di,si
	jae	MC_BACKWARDS
	cmp	cx,#7
	jb	MC_SMALL
	test	si,#1
	je	MC_ALIGNED	| source even, hope target is too
				| else hope target odd so both become even
	movsb
	dec	cx
MC_ALIGNED:
	shr	cx,1
	rep
	movsw
	rcl	cx,1
MC_SMALL:
	rep
	movsb
MC_EXIT:
# if !__CALLER_SAVES__
	mov	si,bx
	mov	di,dx
# endif
	ret

MC_BACKWARDS:
	je	MC_EXIT		| source == target
	add	si,cx
	dec	si		| stupid thing doesnt pre-decrement
	add	di,cx
	dec	di
	std

	cmp	cx,#11
	jb	MCB_SMALL
	test	si,#1
	jne	MCB_ALIGNED
	movsb
	dec	cx
MCB_ALIGNED:
	dec	si		| ptr was right for byte, make it for word
	dec	di
	shr	cx,1
	rep
	movsw
	rcl	cx,1
	inc	si		| back to a byte ptr
	inc	di
MCB_SMALL:
	rep
	movsb
MCB_EXIT:
	cld
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
	mov	esi,TEMPS+_memcpy.s2+4[esp]
	mov	eax,TEMPS+_memcpy.nbytes+4[esp]
# else
	mov	edi,TEMPS+_memcpy.s1[esp]
	mov	esi,TEMPS+_memcpy.s2[esp]
	mov	eax,TEMPS+_memcpy.nbytes[esp]
# endif
	cmp	edi,esi
	jae	MC_BACKWARDS

	cmp	eax,#10		| this was calculated exectly once
	jb	MC_SMALL	| other "SMALL"s are counts of setup instructs
	mov	ecx,esi		| align source, hope target is too
	neg	ecx
	and	ecx,#3		| count for alignment
	sub	eax,ecx
	rep
	movsb
	mov	ecx,eax
	shr	ecx,2		| count of dwords
	rep
	movsd
	and	eax,#3
MC_SMALL:
	xchg	ecx,eax		| remainder
	rep
	movsb
# if __FIRST_ARG_IN_AX__
	pop	eax
# else
	mov	eax,TEMPS+_memcpy.s1[esp]
# endif
# if !__CALLER_SAVES__
	pop	esi
	mov	edi,edx
# endif
	ret

MC_BACKWARDS:
	je	MC_EXIT		| source == target
	lea	esi,-1[esi+eax]	| do predecrement by hand
	lea	edi,-1[edi+eax]
	std

	cmp	eax,#13
	jb	MCB_SMALL
	lea	ecx,[esi+1]
	and	ecx,#3
	sub	ecx,ecx
	rep
	movsb
	sub	esi,#3		| down to a dword ptr
	sub	edi,#3
	mov	ecx,eax
	shr	ecx,2
	rep
	movsd
	add	esi,#3		| back to a byte ptr
	add	edi,#3
	and	eax,#3
MCB_SMALL:
	xchg	ecx,eax
	rep
	movsb
MCB_EXIT:
	cld
MC_EXIT:
# if __FIRST_ARG_IN_AX__
	pop	eax
# else
	mov	eax,TEMPS+_memcpy.s1[esp]
# endif
# if !__CALLER_SAVES__
	pop	esi
	mov	edi,edx
# endif
	
| so much for a special memcpy instruction!

# endasm
#endif /* __AS386_32__ */
#endif /* C_CODE etc */
}
