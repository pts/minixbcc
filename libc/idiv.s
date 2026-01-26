| idiv.s
| idiv_ doesn't preserve edx/dx (returns remainder in it)

	.globl idiv_
	.text
	.even

idiv_:
if __IBITS__ = 32
	cdq
	idiv	ebx
else  | if __IBITS__ = 32
idiv_:
	cwd
	idiv	bx
endif  | else if __IBITS__ = 32
	ret
