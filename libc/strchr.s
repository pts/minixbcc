| strchr.s
|	char *strchr(const char *s, int c)
|
|	Returns location of the first occurrence of c (converted to char)
|	in the string pointed to by s.  Returns NULL if c does not occur.

.define	_strchr
.text
_strchr:
if __IBITS__ = 32
error unimplemented
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	mov	bx,si		| save si
	mov	si,sp
	movb	dl,[si+4]
	mov	si,[si+2]
	cld
	test	si,#1		| align string on word boundary
	jz	word_loop
	lodb
	cmpb	al,dl
	je	one_past
	orb	al,al
	jz	no_match
word_loop:			| look for c word by word
	lodw
	cmpb	al,dl
	je	two_past
	orb	al,al
	jz	no_match
	cmpb	ah,dl
	je	one_past
	orb	ah,ah
	jnz	word_loop
no_match:
	xor	ax,ax
	mov	si,bx		| restore si
	ret
two_past:
	dec	si
one_past:
	dec	si
	mov	ax,si
	mov	si,bx		| restore si
endif
	ret
