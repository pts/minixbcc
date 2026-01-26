| lnegl.s

	.globl	lnegl
	.globl	lnegul
	.text
	.even

lnegl:
lnegul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	neg	bx
	neg	ax
	sbb	bx,*0
endif
	ret
