| idiv.s
| idiv_ doesn't preserve edx (returns remainder in it)

	.globl idiv_
	.text
	.even

idiv_:
	cdq
	idiv	ebx
	ret
