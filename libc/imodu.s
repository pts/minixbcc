| imodu.s
| imodu doesn't preserve edx/dx (returns quotient in it)

	.globl imodu
	.text
	.even

imodu:
if __IBITS__ = 32
	xor	edx,edx
	div	ebx
	mov	eax,edx		| instruction queue full so xchg slower
else  | if __IBITS__ = 32
	xor	dx,dx
	div	bx
	mov	ax,dx		| instruction queue full so xchg slower
endif  | else if __IBITS__ = 32
	ret
