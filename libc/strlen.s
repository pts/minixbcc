| strlen.s
|	size_t strlen(const char *s)
|
|	Returns the length of the string pointed to by s.

.define	_strlen
.text
_strlen:
	mov	bx,di		| save di
	mov	di,sp
	mov	di,[di+2]
	mov	cx,#-1
	xorb	al,al
	cld
	repne
	scab
	not	cx		| silly trick gives length (including null)
	dec	cx		| forget about null
	mov	ax,cx
	mov	di,bx		| restore di
	ret
