| lincb.s

	.globl	lincb
	.globl	lincub
	.text
	.even

lincb:
lincub:
	inc	[bx+2]
	je	LINC_HIGH_WORD
	ret

	.even

LINC_HIGH_WORD:
	inc	[bx]
	ret
