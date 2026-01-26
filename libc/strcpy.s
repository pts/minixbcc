| strcpy.s
|	char *strcpy(char *s1, const char *s2)
|
|	Copy the string pointed to by s2, including the terminating null
|	character, into the array pointed to by s1.  Returns s1.

.define	_strcpy
.text
_strcpy:
if __IBITS__ = 32  | Based on i386 inline assembly code (#asm .. #endasm) in a C source file by Bruce Evans.
.s1 = 4
.s2 = 8
	mov	edx,edi
	push	esi
	mov	esi,4+.s2[esp]
	mov	edi,esi
	sub	eax,eax		| method with finding length of s2 takes
	lea	ecx,-1[eax]	| 18+12n vs 19n
	repnz
	scasb
	inc	ecx
	neg	ecx
	mov	eax,4+.s1[esp]
	mov	edi,eax
	rep			| it is faster to avoid fancy alignment tests
	movsb			| (5+4n vs 28+[0-12]+[1-3]n+[0-12])
	pop	esi		| but could join memcpy
	mov	edi,edx
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	mov	bx,si		| save si and di
	mov	cx,di
	mov	di,sp
	mov	si,[di+4]
	mov	di,[di+2]
	mov	dx,di
	cld
	test	si,#1		| align source on word boundary
	jz	word_copy
	lodb
	stob
	orb	al,al
	jz	exit
word_copy:			| loop to copy words
	lodw
	orb	al,al
	jz	move_last_byte	| early exit if low byte == 0
	stow
	orb	ah,ah
	jnz	word_copy
	jmp	exit
move_last_byte:
	stob			| add odd zero byte
exit:
	mov	ax,dx
	mov	si,bx		| restore si and di
	mov	di,cx
endif
	ret
