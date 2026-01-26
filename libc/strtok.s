| strtok.s
|	char *strtok(char *s1, const char *s2)
|
|	Returns a pointer to the "next" token in s1.  Tokens are 
|	delimited by the characters in the string pointed to by s2.

.define	_strtok
.data
scan:	.word	0  | !! Move it to .bss with .comm.
.text
_strtok:
if __IBITS__ = 32
error unimplemented
else  | Based on i86 (8086) to-be-preprocessed assembly source file /usr/src/lib/string/*.x . Patched by Bruce Evans.
	push	bp
	mov	bp,sp
	push	si
	push	di
	cld
	mov	bx,[bp+4]
	or	bx,bx		| if s != NULL,
	jnz	s2_length	|   we start a new string
	mov	bx,[scan]
	or	bx,bx		| if old string exhausted,
	jz	exit		|   exit early
s2_length:			| find length of s2
	mov	di,[bp+6]
	mov	cx,#-1
	xorb	al,al
	repne
	scab
	not	cx
	dec	cx
	jz	string_finished	| if s2 has length zero, we are done
	mov	dx,cx		| save length of s2

	mov	si,bx
	xor	bx,bx		| return value is NULL
delim_loop:			| dispose of leading delimiters
	lodb
	orb	al,al
	jz	string_finished
	mov	di,[bp+6]
	mov	cx,dx
	repne
	scab
	je	delim_loop

	lea	bx,[si-1]	| return value is start of token
token_loop:			| find end of token
	lodb
	orb	al,al
	jz	string_finished
	mov	di,[bp+6]
	mov	cx,dx
	repne
	scab
	jne	token_loop
	movb	[si-1],*0	| terminate token
	mov	[scan],si	| set up for next call
	jmp	exit
string_finished:
	mov	[scan],#0	| ensure NULL return in future
exit:
	mov	ax,bx
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
endif
	ret
