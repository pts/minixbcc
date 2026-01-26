| ldivul.s
| unsigned bx:ax / [di+2]:[di], quotient bx:ax,remainder di:cx, dx not preserved

	.globl	ldivul
	.extern	ludivmod
	.text
	.even

ldivul:
if __IBITS__ = 32
error unneeded
else  | Based on assembly source file (*.s) by Bruce Evans.
	mov	cx,[di]
	mov	di,[di+2]
	call	ludivmod	| unsigned bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,cx
	xchg	bx,di
endif
	ret
