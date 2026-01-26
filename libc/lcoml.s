| lcoml.s

	.globl	lcoml
	.globl	lcomul
	.text
	.even

lcoml:
lcomul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	not	ax
	not	bx
endif
	ret
