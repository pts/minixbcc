| lmodl.s
| bx:ax % [di+2]:[di], remainder bx:ax, quotient di:cx, dx not preserved

	.globl	lmodl
	.extern	ldivmod
	.text
	.even

lmodl:
if __IBITS__ = 32
error unneeded
	ret
else  | Based on assembly source file (*.s) by Bruce Evans.
	mov	cx,[di]
	mov	di,[di+2]
	jmp	ldivmod		| bx:ax / di:cx, quot di:cx, rem bx:ax
endif
