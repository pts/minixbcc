| imodu.s
| imodu doesn't preserve edx (returns quotient in it)

	.globl imodu
	.text
	.even

imodu:
	xor	edx,edx
	div	ebx
	mov	eax,edx		| instruction queue full so xchg slower
	ret
