| landl.s

	.globl	landl
	.globl	landul
	.text
	.even

landl:
landul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	and	ax,[di]
	and	bx,[di+2]
endif
	ret
