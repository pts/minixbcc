| sendrec.s
| int send(int src_dest, message *m_ptr);
| int receive(int src_dest, message *m_ptr);
| int sendrec(int src_dest, message *m_ptr);

| These functions preserve all registers except eax, ecx, edx and eflags.

	.define _send
	.define	_receive
	.define	_sendrec

FIRSTARGINREG	=	0		| nonzero if first arg is in eax

SEND		=	1
RECEIVE		=	2
SENDREC		=	3
SYS386VEC	=	33

if	FIRSTARGINREG
	M_PTR_OFF	=	4+4
else
	SRC_DEST_OFF	=	4+4
	M_PTR_OFF	=	4+4+4
endif

	.text
	.align	4
_send:
	push	ebx
if	FIRSTARGINREG = 0
	mov	eax,SRC_DEST_OFF[esp]	| src_dest
endif
	mov	ebx,M_PTR_OFF[esp]	| m_ptr
	mov	ecx,#SEND		| sys_call(SEND, src_dest, message)
	int	SYS386VEC		| trap to the kernel
	pop	ebx
	ret

	.align	4
_receive:
	push	ebx
if	FIRSTARGINREG = 0
	mov	eax,SRC_DEST_OFF[esp]
endif
	mov	ebx,M_PTR_OFF[esp]
	mov	ecx,#RECEIVE		| sys_call(RECEIVE, src_dest, message)
	int	SYS386VEC
	pop	ebx
	ret

	.align	4
_sendrec:
	push	ebx
if	FIRSTARGINREG = 0
	mov	eax,SRC_DEST_OFF[esp]
endif
	mov	ebx,M_PTR_OFF[esp]
	mov	ecx,#SENDREC		| sys_call(SENDREC, src_dest, message)
	int	SYS386VEC
	pop	ebx
	ret
