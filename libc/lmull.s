| lmull.s
| lmull, lmulul don't preserve cx, dx

	.globl	lmull
	.globl	lmulul
	.text
	.even

lmull:
lmulul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	mov	cx,ax
	mul	word ptr [di+2]
	xchg	ax,bx
	mul	word ptr [di]
	add	bx,ax
	mov	ax,ptr [di]
	mul	cx
	add	bx,dx
endif
	ret
