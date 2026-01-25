if __IBITS__ = 32
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
else  | if __IBITS__ = 32
| imodu.s
| imodu doesn't preserve dx (returns quotient in it)

	.globl imodu
	.text
	.even

imodu:
	xor	dx,dx
	div	bx
	mov	ax,dx		| instruction queue full so xchg slower
	ret
endif  | else if __IBITS__ = 32
