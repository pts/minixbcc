| catchsig.s

	.define _begsig

FIRSTARGINREG	=	0		| nonzero if first arg is in eax

MTYPE		=	4		| offset of m_type in message struct
PSIZE		=	4		| size of pointers in functions

	.text
	.align	4
_begsig:
	pushad				| save registers (32 bytes)
	push	ds			| 2 bytes each seg reg
	push	es
	push	fs
	push	gs
	push	dword [__M+MTYPE]	| save status of last system call
	mov	eax,32+2+2+2+2+4[esp]	| signal number from stack before save
if FIRSTARGINREG = 0
	push	eax			| is argument to signal handler
endif
	call	___vectab-PSIZE[eax*PSIZE]	| index is signal number - 1
if FIRSTARGINREG = 0
	pop	eax			| discard signal number argument
endif
	pop	dword [__M+MTYPE]	| restore status of last system call
	pop	gs			| restore registers
	pop	fs
	pop	es
	pop	ds
	popad
	add	esp,#4			| discard signal number again
	iretd

	.data
	.extern	___vectab
	.extern	__M
