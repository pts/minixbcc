| idivu.s
| idiv_u doesn't preserve edx (returns remainder in it)

	.globl idiv_u
	.text
	.even

idiv_u:
	xor	edx,edx
	div	ebx
	ret
