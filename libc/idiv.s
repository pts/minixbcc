if __IBITS__ = 32
| idiv.s
| idiv_ doesn't preserve edx (returns remainder in it)

	.globl idiv_
	.text
	.even

idiv_:
	cdq
	idiv	ebx
	ret
else  | if __IBITS__ = 32
| idiv.s
| idiv_ doesn't preserve dx (returns remainder in it)

	.globl idiv_
	.text
	.even

idiv_:
	cwd
	idiv	bx
	ret
endif  | else if __IBITS__ = 32
