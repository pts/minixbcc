| This is the C run-time start-off routine.
| Its job is to take the arguments as put on the stack by EXEC, and to set
| them up the way _main expects them.

.define	crtso
.extern _main, _exit

if __IBITS__ = 32
	.align	4
crtso:
	sub	ebp,ebp			| clear for backtrace of core files
	mov	eax,[esp]		| argc
	lea	edx,4[esp]		| argv
	lea	ecx,8[esp+eax*4]	| envp
	mov	[_environ],ecx
	push	ecx
	push	edx
	push	eax
	call	_main
	push	eax			| exit status
	call	_exit
crt_oops:
	j	crt_oops		| exit can fail when break < message

	.data
	.align	4
	.zerow	8			| food for null pointer bugs
	.bss
	.comm	_environ,4
else  | if __IBITS__ = 32
.globl _environ
.globl begtext, begdata, begbss  | !! Are these needed?
begtext:
crtso:
		sub	bp,bp	| clear for backtrace of core files
		mov	bx,sp
		mov	cx,[bx]
		add	bx,*2
		mov	ax,cx
		inc	ax
		shl	ax,#1
		add	ax,bx
		mov	[_environ],ax	| save envp in environ
		push	ax	| push environ
		push	bx	| push argv
		push	cx	| push argc
		call	_main
		add	sp,*6
		push	ax	| push exit status
		call	_exit

.data
begdata:
		.zerow 8	| food for null pointer bugs
_environ:	.word 0  | !! Move to .bss with .comm.
.bss
begbss:
endif  | else if __IBITS__ = 32
