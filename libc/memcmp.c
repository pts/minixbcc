/* memcmp.c
 *	int memcmp(const void *s1, const void *s2, size_t n)
 *
 *	Compares the first n (unsigned) characters of the objects pointed
 *	to by s1 and s2.  Returns zero if all characters are identical, a
 *	positive number if s1 greater than s2, a negative number otherwise.
 */

#include <string.h>

int memcmp(s1, s2, n)
_CONST void *s1;
_CONST void *s2;
size_t n;
{
#if C_CODE || !__AS386_32__ || __FIRST_ARG_IN_AX__
    register _CONST unsigned char *cs1;
    register _CONST unsigned char *cs2;

    for (cs1 = s1, cs2 = s2; n-- != 0; ++cs1, ++cs2)
	if (*cs1 != *cs2)
	    return *cs1 - *cs2;
    return 0;
    
#else /* !C_CODE etc */
# asm
BYTE_LIMIT	=	7	| if n is above this, work with dwords
	push	esi
	push	edi
	mov	esi,4+4+4[esp]
	mov	edi,4+4+4+4[esp]
	mov	edx,4+4+4+4+4[esp]
	sub	eax,eax		| provisional return value
	cmp	esi,edi
	je	exit		| early exit if s1 == s2
	cmp	edx,#BYTE_LIMIT
	jbe	byte_compare
	mov	ecx,esi		| align source, hope target is too
	neg	ecx
	and	ecx,#3		| count for alignment
	sub	edx,ecx		| remainder
	cmp	ecx,ecx		| set equals flags in case rep is null
				| to avoid jump in likely aligned case
	rep
	cmpsb
	jnz	result_in_flags
	mov	ecx,edx
	and	edx,#3		| new remainder
	shr	ecx,#2		| count of dwords (known to be nonzero)
	rep
	cmpsd
	jz	byte_compare
	mov	edx,#4		| new remainder
	sub	esi,edx		| back to the dword with the difference
	sub	edi,edx
byte_compare:
	xchg	ecx,edx		| remainder in ecx, trash in edx
	cmp	ecx,ecx		| avoid jump as above
	rep
	cmpsb
result_in_flags:
	seta	al		| eax = 1 if s1 > s2, else 0 (eax was 0)
	setb	cl		| ecx = 1 if s1 < s2, else 0 (ecx was 0-3)
	sub	eax,ecx
exit:
	pop	edi
	pop	esi
	ret
# endasm
#endif /* C_CODE etc */
}
