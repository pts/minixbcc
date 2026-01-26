| imod.s
| imod doesn't preserve edx/dx (returns quotient in it)

	.globl imod
	.text
	.even

imod:
if __IBITS__ = 32
	cdq
	idiv	ebx
	mov	eax,edx		| instruction queue full so xchg slower
else  | if __IBITS__ = 32
	cwd
	idiv	bx
	mov	ax,dx		| instruction queue full so xchg slower
endif  | else if __IBITS__ = 32
	ret
