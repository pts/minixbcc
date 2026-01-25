| ldecb.s

	.globl	ldecb
	.globl	ldecub
	.text
	.even

ldecb:
ldecub:
	cmp	[bx+2],*0
	je	LDEC_BOTH
	dec	[bx+2]
	ret

	.even

LDEC_BOTH:
	dec	[bx+2]
	dec	[bx]
	ret
