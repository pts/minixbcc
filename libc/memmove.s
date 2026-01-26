| memmove.s
|	void *memmove(void *s1, const void *s2, size_t n)
|	void *memcpy(void *s1, const void *s2, size_t n)
|
|	Copy n characters from the object pointed to by s2 into the
|	object pointed to by s1.  Copying takes place as if the n
|	characters pointed to by s2 are first copied to a temporary
|	area and then copied to the object pointed to by s1.
|
|	Per X3J11, memcpy may have undefined results if the objects
|	overlap; since the performance penalty is insignificant, we
|	use the safe memmove code for it as well.
|
|    i86 calling conventions (small model only):
|	arguments on stack except 1st arg may be in ax
|	direction flag clear
|	es == ds
|	routine must preserve ds, es and direction
|	may have to preserve si, di
|	return result in ax
|

.define	_memmove, _memcpy
.text
_memmove:
_memcpy:
.s1 = 4
.s2 = 8
.nbytes = 12
if __IBITS__ = 32  | Based on i386 inline assembly code (#asm .. #endasm) in a C source file by Bruce Evans.
	mov	edx,edi
	push	esi
	mov	edi,4+.s1[esp]
	mov	esi,4+.s2[esp]
	mov	eax,4+.nbytes[esp]
	cmp	edi,esi
	jae	.MC_BACKWARDS
	cmp	eax,#10		| this was calculated exectly once
	jb	.MC_SMALL	| other "SMALL"s are counts of setup instructs
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
.MC_SMALL:
	xchg	ecx,eax		| remainder
	rep
	movsb
	mov	eax,4+.s1[esp]
	pop	esi
	mov	edi,edx
	ret
.MC_BACKWARDS:
	je	.MC_EXIT		| source == target
	lea	esi,-1[esi+eax]	| do predecrement by hand
	lea	edi,-1[edi+eax]
	std
	cmp	eax,#13
	jb	.MCB_SMALL
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
.MCB_SMALL:
	xchg	ecx,eax
	rep
	movsb
	cld
.MC_EXIT:
	mov	eax,4+.s1[esp]
	pop	esi
	mov	edi,edx
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	mov	bx,si		| save si and di
	mov	dx,di
	mov	di,sp
	mov	cx,[di+6]
	mov	si,[di+4]
	mov	di,[di+2]
	mov	ax,di		| save a copy of s1
	jcxz	exit		| early exit if n == 0
	sub	di,si
	je	exit		| early exit if s1 == s2
	jb	left_to_right	| left to right if s1 < s2
	cmp	di,cx
	jae	left_to_right	| left to right if no overlap
right_to_left:
	mov	di,ax		| retrieve s1
	std
	add	si,cx		| compute where objects end
	dec	si
	add	di,cx
	dec	di
	cmp	cx,#10  | BYTE_LIMIT = 10. If n is above this, work with words.
	jbe	byte_move
	test	si,#1		| align source on word boundary
	jnz	word_unaligned
	movb
	dec	cx
word_unaligned:
	dec	si		| adjust to word boundary
	dec	di
	shr	cx,#1		| move words, not bytes
	rep
	movw
	jnc	exit
	inc	si		| fix up addresses for right to left moves
	inc	di
	movb			| move leftover byte
	jmp	exit
left_to_right:
	mov	di,ax		| retrieve s1
	cld
	cmp	cx,#10  | BYTE_LIMIT = 10. If n is above this, work with words.
	jbe	byte_move
	test	si,#1		| align source on word boundary
	jz	word_move
	movb
	dec	cx
word_move:
	shr	cx,#1		| move words, not bytes
	rep
	movw
	adc	cx,cx		| set up to move leftover byte
byte_move:
	rep
	movb
exit:
	cld			| restore direction flag
	mov	si,bx		| restore si and di
	mov	di,dx
endif
	ret
