| ldecl.s

	.globl	ldecl
	.globl	ldecul
	.text
	.even

ldecl:
ldecul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	cmp	word ptr [bx],*0
	je	LDEC_BOTH
	dec	word ptr [bx]
	ret

	.even

LDEC_BOTH:
	dec	word ptr [bx]
	dec	word ptr [bx+2]
endif
	ret
