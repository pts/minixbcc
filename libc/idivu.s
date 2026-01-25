if __IBITS__ = 32
| idivu.s
| idiv_u doesn't preserve edx (returns remainder in it)

	.globl idiv_u
	.text
	.even

idiv_u:
	xor	edx,edx
	div	ebx
	ret
else  | if __IBITS__ = 32
| idivu.s
| idiv_u doesn't preserve dx (returns remainder in it)

	.globl idiv_u
	.text
	.even

idiv_u:
	xor	dx,dx
	div	bx
	ret
endif  | else if __IBITS__ = 32
