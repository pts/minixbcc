| lincl.s

	.globl	lincl
	.globl	lincul
	.text
	.even

lincl:
lincul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	inc	word ptr [bx]
	je	LINC_HIGH_WORD
	ret

	.even

LINC_HIGH_WORD:
	inc	word ptr [bx+2]
endif
	ret
