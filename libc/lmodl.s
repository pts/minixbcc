| lmodl.s
| bx:ax % 2(di):(di), remainder bx:ax, quotient di:cx, dx not preserved

	.globl	lmodl
	.extern	ldivmod
	.text
	.even

lmodl:
	mov	cx,(di)
	mov	di,2(di)
	jmp	ldivmod		| bx:ax / di:cx, quot di:cx, rem bx:ax
