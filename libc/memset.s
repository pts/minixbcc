| memset.s
|	void *memset(void *s, int c, size_t n)
|
|	Copies the value of c (converted to unsigned char) into the
|	first n locations of the object pointed to by s.

.define	_memset
.text
_memset:
	push	di
	mov	di,sp
	mov	cx,8(di)
	movb	al,6(di)
	mov	di,4(di)
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
	ret
