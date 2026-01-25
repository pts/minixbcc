| memchr.s
|	void *memchr(const void *s, int c, size_t n)
|
|	Returns a pointer to the first occurrence of c (converted to
|	unsigned char) in the object pointed to by s, NULL if none.

.define	_memchr
.text
_memchr:
	mov	bx,di		| save di
	mov	di,sp
	xor	dx,dx		| default result is NULL
	mov	cx,[di+6]
	jcxz	exit		| early exit if n == 0
	movb	al,[di+4]
	mov	di,[di+2]
	cld
	repne
	scab
	jne	exit
	dec	di
	mov	dx,di
exit:
	mov	di,bx		| restore di
	mov	ax,dx
	ret
