| lmulb.s
| lmulb, lmulub don't preserve cx, dx

	.globl	lmulb
	.globl	lmulub
	.text
	.even

lmulb:
lmulub:
	mul	[di+2]
	xchg	ax,bx
	mov	cx,ax
	mul	[di]
	add	bx,ax
	mov	ax,[di+2]
	mul	cx
	add	bx,dx
	xchg	ax,bx
	ret
