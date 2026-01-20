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
