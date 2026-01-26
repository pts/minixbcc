| lsubl.s

	.globl	lsubl
	.globl	lsubul
	.text
	.even

lsubl:
lsubul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	sub	ax,[di]
	sbb	bx,[di+2]
endif
	ret
