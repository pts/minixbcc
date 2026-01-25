| ldivl.s
| bx:ax / [di+2]:[di], quotient bx:ax, remainder di:cx, dx not preserved

	.globl	ldivl
	.extern	ldivmod
	.text
	.even

ldivl:
	mov	cx,[di]
	mov	di,[di+2]
	call	ldivmod		| bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,cx
	xchg	bx,di
	ret

