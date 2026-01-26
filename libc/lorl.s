| lorl.s

	.globl	lorl
	.globl	lorul
	.text
	.even

lorl:
lorul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	or	ax,[di]
	or	bx,[di+2]
endif
	ret
