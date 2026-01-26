| memcmp.s
|	int memcmp(const void *s1, const void *s2, size_t n)
|
|	Compares the first n characters of the objects pointed to by
|	s1 and s2.  Returns zero if all characters are identical, a
|	positive number if s1 greater than s2, a negative number otherwise.

.define	_memcmp
.text
_memcmp:
if __IBITS__ = 32  | Based on i386 inline assembly code (#asm .. #endasm) in a C source file by Bruce Evans.
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
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	mov	bx,sp
	push	si
	push	di
	xor	ax,ax		| default return is equality
	mov	cx,[bx+6]
	jcxz	exit		| early exit if n == 0
	mov	si,[bx+2]
	mov	di,[bx+4]
	cmp	si,di
	je	exit		| early exit if s1 == s2
	cld
	cmp	cx,*10  | BYTE_LIMIT = 10. If n is above this, work with words.
	ja	word_compare
byte_compare:
	repe
	cmpb
	jne	one_past_mismatch
	pop	di
	pop	si
	ret
word_compare:
	test	si,#1		| align s1 on word boundary
	jz	word_aligned
	cmpb
	jne	one_past_mismatch
	dec	cx
word_aligned:
	mov	dx,cx		| save count
	shr	cx,#1		| compare words, not bytes
	jz	almost_done
	repe
	cmp
	je	almost_done
	sub	si,#2
	sub	di,#2
	cmpb
	jne	one_past_mismatch
	jmp	at_mismatch
almost_done:
	test	dx,#1
	jz	exit
	jmp	at_mismatch
one_past_mismatch:
	dec	si
	dec	di
at_mismatch:
	xorb	ah,ah
	movb	al,[si]
	subb	al,[di]
	sbbb	ah,ah
exit:
	pop	di
	pop	si
endif
	ret
