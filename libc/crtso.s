| This is the C run-time start-off routine.
| Its job is to take the arguments as put on the stack by EXEC, and to set
| them up the way _main expects them.

	.define	crtso
	.extern	_exit
	.extern	_main

	.text
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
