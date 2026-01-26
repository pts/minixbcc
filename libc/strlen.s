| strlen.s
|	size_t strlen(const char *s)
|
|	Returns the length of the string pointed to by s.

.define	_strlen
.text
_strlen:
if __IBITS__ = 32  | Based on i386 inline assembly code (#asm .. #endasm) in a C source file by Bruce Evans.
.s = 4
	push	edi
	mov	edi,4+.s[esp]
	mov	edx,edi
	sub	eax,eax
	lea	ecx,-1[eax]
	repnz
	scasb
	lea	eax,-1[edi]
	sub	eax,edx
	pop	edi
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
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
endif
	ret
