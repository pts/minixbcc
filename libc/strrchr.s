| strrchr.s
|	char *strrchr(const char *s, int c)
|
|	Locates final occurrence of c (as unsigned char) in string s.

.define	_strrchr
.text
_strrchr:
	mov	bx,di		| save di
	mov	di,sp
	xor	dx,dx		| default result is NULL
	movb	ah,[di+4]
	mov	di,[di+2]
	cld
	mov	cx,#-1		| find end of string
	xorb	al,al
	repne
	scab
	not	cx		| silly trick gives length (including null)
	dec	di		| point back at null character
	movb	al,ah		| find last occurrence of c
	std
	repne
	scab
	jne	exit
	inc	di
	mov	dx,di
exit:
	cld			| clear direction flag
	mov	di,bx		| restore di
	mov	ax,dx
	ret
