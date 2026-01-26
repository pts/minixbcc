| memset.s
|	void *memset(void *s, int c, size_t n)
|
|	Copies the value of c (converted to unsigned char) into the
|	first n locations of the object pointed to by s.

.define	_memset
.text
_memset:
if __IBITS__ = 32  | Based on i386 inline assembly code (#asm .. #endasm) in a C source file by Bruce Evans.
.s = 4
.ch = 8
.n = 12
	push	edi
	mov	edi,4+.s[esp]
	mov	al,4+.ch[esp]
	mov	ecx,4+.n[esp]
	cmp	ecx,#17
	jb	.MS_SMALL
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
	push	esi
	mov	esi,edi		| set up for movs which is 1 cycle faster
	stosd
	dec	ecx
	rep
	movsd			| propagate 1st zero dword
	pop	esi
	and	edx,#3
	mov	ecx,edx
.MS_SMALL:
	rep
	stosb
	mov	eax,4+.s[esp]
	pop	edi
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	push	di
	mov	di,sp
	mov	cx,[di+8]
	movb	al,[di+6]
	mov	di,[di+4]
	mov	bx,di		| return value is s
	jcxz	exit		| early exit if n == 0
	cld
	cmp	cx,*10  | BYTE_LIMIT = 10. If n is above this, work with words.
	jbe	byte_set
	movb	ah,al		| set up second byte
	test	di,#1		| align on word boundary
	jz	word_aligned
	stob
	dec	cx
word_aligned:
	shr	cx,#1		| set words, not bytes
	rep
	stow
	adc	cx,cx		| set up to set leftover byte
byte_set:
	rep
	stob
exit:
	pop	di
	mov	ax,bx
endif
	ret
