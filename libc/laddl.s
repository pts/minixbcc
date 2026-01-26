| laddl.s

	.globl	laddl
	.globl	laddul
	.text
	.even

laddl:
laddul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	add	ax,[di]
	adc	bx,[di+2]
endif
	ret
