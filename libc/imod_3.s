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
