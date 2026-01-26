| idivu.s
| idiv_u doesn't preserve edx/dx (returns remainder in it)

	.globl idiv_u
	.text
	.even

idiv_u:
if __IBITS__ = 32
	xor	edx,edx
	div	ebx
else  | if __IBITS__ = 32
	xor	dx,dx
	div	bx
endif  | else if __IBITS__ = 32
	ret
