| lincl.s

	.globl	lincl
	.globl	lincul
	.text
	.even

lincl:
lincul:
	inc	word ptr (bx)
	je	LINC_HIGH_WORD
	ret

	.even

LINC_HIGH_WORD:
	inc	word ptr 2(bx)
	ret
