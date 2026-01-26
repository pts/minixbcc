.define _begsig

if __IBITS__ = 32
FIRSTARGINREG	=	0		| nonzero if first arg is in eax

MTYPE		=	4		| offset of m_type in message struct
PSIZE		=	4		| size of pointers in functions

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
else  | if __IBITS__ = 32
mtype = 2			| _M+mtype = &_M.m_type
_begsig:
	push ax			| after interrupt, save all regs
	push bx
	push cx
	push dx
	push si
	push di
	push bp
	push ds
	push es
	mov bx,sp
	mov bx,[bx+18]		| bx = signal number
	mov ax,bx		| ax = signal number
	dec bx			| __vectab[0] is for sig 1
	add bx,bx		| pointers are two bytes on 8088
	mov bx,[bx+___vectab]	| bx = address of routine to call
	push [__M+mtype]	| push status of last system call
	push ax			| func called with signal number as arg
	call bx
back:
	pop ax			| get signal number off stack
	pop [__M+mtype]		| restore status of previous system call
	pop es			| signal handling finished
	pop ds
	pop bp
	pop di
	pop si
	pop dx
	pop cx
	pop bx
	pop ax
	add sp,*2		| remove signal number from stack
	iret

.data 
.extern ___vectab, __M
endif  | else if __IBITS__ = 32
