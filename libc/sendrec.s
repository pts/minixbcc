| sendrec.s
| int send(int src_dest, message *m_ptr);
| int receive(int src_dest, message *m_ptr);
| int sendrec(int src_dest, message *m_ptr);

| !! Is this true? Sometimes they ruin EBX. On i386, these functions preserve all registers except EAX, ECX, EDX and EFLAGS.
| On i86, send(), receive(), sendrec() preserve all registers except AX, BX, CX, DX and FLAGS.

.define _send, _receive, _sendrec

| See ../h/com.h for C definitions
SEND = 1
RECEIVE = 2
SENDREC = 3
if __IBITS__ = 32
FIRSTARGINREG = 0  | nonzero if first arg is in eax
SYS386VEC = 33
else
SYSVEC = 32
endif

if __IBITS__ = 32
if	FIRSTARGINREG
	M_PTR_OFF	=	4+4
else
	SRC_DEST_OFF	=	4+4
	M_PTR_OFF	=	4+4+4
endif
endif

|*========================================================================*
|                           send and receive                              *
|*========================================================================*
.globl _send
if __IBITS__ = 32
	.align	4  | !! Not needed.
endif
_send:  | !! Remove this function if unused in the libc.
if __IBITS__ = 32
	push	ebx
if	FIRSTARGINREG = 0
	mov	eax,SRC_DEST_OFF[esp]	| src_dest
endif
	mov	ebx,M_PTR_OFF[esp]	| m_ptr
	mov	ecx,#SEND		| sys_call(SEND, src_dest, message)
	int	SYS386VEC		| trap to the kernel
	pop	ebx
else
	pop dx			| return addr
	pop ax			| dest-src
	pop bx			| message pointer
	sub sp,*4
	push dx
	mov cx,*SEND		| send(dest, ptr)
	int SYSVEC		| trap to the kernel
endif
	ret

.globl _receive
if __IBITS__ = 32
	.align	4  | !! Not needed.
endif
_receive:  | !! Remove this function if unused in the libc.
if __IBITS__ = 32
	push	ebx
if	FIRSTARGINREG = 0
	mov	eax,SRC_DEST_OFF[esp]
endif
	mov	ebx,M_PTR_OFF[esp]
	mov	ecx,#RECEIVE		| sys_call(RECEIVE, src_dest, message)
	int	SYS386VEC
	pop	ebx
else
	pop dx
	pop ax
	pop bx
	sub sp,*4
	push dx
	mov cx,*RECEIVE		| receive(src, ptr)
	int SYSVEC		| trap to the kernel
endif
	ret

.globl _sendrec
if __IBITS__ = 32
	.align	4  | !! Not needed.
endif
_sendrec:
if __IBITS__ = 32
	push	ebx
if	FIRSTARGINREG = 0
	mov	eax,SRC_DEST_OFF[esp]
endif
	mov	ebx,M_PTR_OFF[esp]
	mov	ecx,#SENDREC		| sys_call(SENDREC, src_dest, message)
	int	SYS386VEC
	pop	ebx
else
	pop dx
	pop ax
	pop bx
	sub sp,*4
	push dx
	mov cx,*SENDREC		| sendrec(srcdest, ptr)
	int SYSVEC		| trap to the kernel
endif
	ret
