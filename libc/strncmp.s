| strncmp.s
|	int strncmp(const char *s1, const char *s2, size_t n)
|
|	Compares up to n characters from the strings pointed to by s1
|	and s2.  Returns zero if the (possibly null terminated) arrays
|	are identical, a positive number if s1 is greater than s2, and
|	a negative number otherwise.

.define	_strncmp
.text
_strncmp:
if __IBITS__ = 32  | Based on i386 inline assembly code (#asm .. #endasm) in a C source file by Bruce Evans.
.s1 = 4
.s2 = 8
.n = 12
	mov	edx,edi
	push	esi
	mov	esi,4+.s1[esp]
	mov	edi,4+.s2[esp]
	mov	ecx,4+.n[esp]
	sub	eax,eax		| prepare for various exits
	jecxz	.SNCMP_1EXIT
.SNCMP_LOOP:
	lodsb
	or	al,al
	je	.SNCMP_EXIT
	scasb
	loopz	.SNCMP_LOOP
	dec	edi
.SNCMP_EXIT:
	sub	al,[edi]
	sbb	ah,#0
	cwde
.SNCMP_1EXIT:
	pop	esi
	mov	edi,edx
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	mov	bx,sp
	push	si
	push	di
	xor	ax,ax		| default result is equality
	mov	cx,[bx+6]
	jcxz	exit		| early exit if n == 0
	mov	si,[bx+2]
	mov	di,[bx+4]
	cmp	si,di
	je	exit		| early exit if s1 == s2
	cld
	test	si,#1		| align s1 on word boundary
	jz	setup_loop
	lodb
	orb	al,al
	jz	last_byte_test
	cmpb	al,[di]
	jne	last_byte_test
	xor	ax,ax
	dec	cx
	jz	exit		| early exit if n == 1
	inc	di
setup_loop:
	mov	dx,cx		| save count
	shr	cx,#1		| work with words, not bytes
	jz	fetch_last_byte
	sub	di,#2		| set up for faster loop
word_loop:			| loop through string by words
	lodw
	add	di,#2
	orb	al,al
	jz	last_byte_test
	cmp	ax,[di]
	jne	find_mismatch
	orb	ah,ah
	loopnz	word_loop
	mov	ax,#0		| zero return value (without setting flags)
	jz	exit
	test	dx,#1		| check for odd byte at end
	jz	exit
	add	di,#2
fetch_last_byte:
	movb	al,[si]
	jmp	last_byte_test
find_mismatch:			| check word for mismatched byte
	cmpb	al,[di]
	jne	last_byte_test
	movb	al,ah
	inc	di
last_byte_test:			| Expects: (al)=char of s1; [di]->char of s2
	xorb	ah,ah
	subb	al,[di]
	sbbb	ah,ah
exit:
	pop	di
	pop	si
endif
	ret
