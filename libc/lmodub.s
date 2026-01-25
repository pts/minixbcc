| lmodub.s
| unsigned ax:bx / [di]:[di+2], remainder ax:bx,quotient cx:di, dx not preserved

	.globl	lmodub
	.extern	ludivmod
	.text
	.even

lmodub:
	xchg	ax,bx
	mov	cx,[di+2]
	mov	di,[di]
	call	ludivmod	| unsigned bx:ax / di:cx, quot di:cx, rem bx:ax
	xchg	ax,bx
	xchg	cx,di
	ret
