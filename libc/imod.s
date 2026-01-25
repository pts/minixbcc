if __IBITS__ = 32
| imod.s
| imod doesn't preserve edx (returns quotient in it)

	.globl imod
	.text
	.even

imod:
	cdq
	idiv	ebx
	mov	eax,edx		| instruction queue full so xchg slower
	ret
else  | if __IBITS__ = 32
| imod.s
| imod doesn't preserve dx (returns quotient in it)

	.globl imod
	.text
	.even

imod:
	cwd
	idiv	bx
	mov	ax,dx		| instruction queue full so xchg slower
	ret
endif  | else if __IBITS__ = 32
